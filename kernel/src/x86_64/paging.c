#include "auxfuncs.h"
#include "auxv.h"
#include "location.h"
#include <memory.h>
#include <paging.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

page_table_t *kernel_pml4t;

static void clone_page_table_level(page_table_t *orig_table,
                                   page_table_t *new_table, int level) {
    for (int idx = 0; idx < 512; idx++) {
        uint64_t entry = orig_table->entries[idx];
        if (entry & 1) {
            // entry is present, now figure out if we need to recurse
            if ((level > 1) && ((entry & 0b10000000) == 0)) {
                page_table_t *orig_lower_table =
                    (page_table_t *)((entry & 0x000FFFFF'FFFFF000) +
                                     getauxval(AT_KXINIX_HHDM_OFFSET).a_val);
                page_table_t *new_lower_table =
                    aligned_alloc(0x1000, sizeof(page_table_t));
                uint64_t new_lower_table_phys =
                    (uint64_t)(new_lower_table)-getauxval(AT_KXINIX_HHDM_OFFSET)
                        .a_val;
                new_table->entries[idx] =
                    (entry & 0x80000000'00000FFF) |
                    (new_lower_table_phys & 0x000FFFFF'FFFFF000);
                clone_page_table_level(orig_lower_table, new_lower_table,
                                       level - 1);
            } else {
                // page specifies a physical address, no cloning needed.
                // TODO: mark page as used in pmm
                new_table->entries[idx] = entry;
            }
        } else {
            new_table->entries[idx] = entry;
        }
    }
}

page_table_t *clone_page_table() {
    uint64_t cr3;
    uint64_t cr4;
    __asm__("mov %%cr3, %0\n"
            "mov %%cr4, %1"
            : "=r"(cr3), "=r"(cr4));
    printf("Old CR3 = %#.16llX\r\n", cr3);

    page_table_t *orig_pml4t =
        (page_table_t *)((cr3 & ~0xFFFLL) +
                         getauxval(AT_KXINIX_HHDM_OFFSET).a_val);
    page_table_t *new_pml4t = aligned_alloc(0x1000, sizeof(page_table_t));
    clone_page_table_level(orig_pml4t, new_pml4t, 4);

    uint64_t new_pml4t_phys =
        (uint64_t)(new_pml4t)-getauxval(AT_KXINIX_HHDM_OFFSET).a_val;
    cr3 = (new_pml4t_phys & ~0xFFFLL) | (cr3 & 0xFFF);
    printf("New CR3 = %#.16llX\r\n", cr3);
    __asm__("mov %0, %%cr3" : : "r"(cr3) : "memory");
    kernel_pml4t = new_pml4t; // TODO: this is poor abstraction
    return new_pml4t;
}

static void set_flags(uint64_t *entry_p, mem_flags_t mem_flags,
                      bool stay_permissive) {
    if ((mem_flags & PROT_EXEC) || stay_permissive) {
        *entry_p &= ~0x80000000'00000000;
    } else {
        *entry_p |= 0x80000000'00000000;
    }
    if ((mem_flags & PROT_WRITE) || stay_permissive) {
        *entry_p |= 0x2;
    } else {
        *entry_p &= ~0x2;
    }
}

#define ADDR_SIGN_EXTEND(addr)                                                 \
    (((addr) & 0x00080000'00000000) ? ((addr) | 0xFFF00000'00000000)           \
                                    : ((addr) & 0x000FFFFF'FFFFFFFF))

void *add_to_hhdm(page_table_t *page_table, uint64_t phys_addr,
                  page_granularity_t granularity, mem_flags_t mem_flags) {
    int cur_level = 4;
    uint64_t hhdm_off = getauxval(AT_KXINIX_HHDM_OFFSET).a_val;
    uint64_t virt_addr = phys_addr + hhdm_off;
    while (1) {
        int entry_idx = (virt_addr >> (cur_level * 9 + 3)) & 0x1FF;
        uint64_t *entry_p = &page_table->entries[entry_idx];
        bool is_present = (*entry_p & 1) != 0;
        bool level_matches_granularity = cur_level == (granularity + 1);
        bool can_descend = cur_level > 1 && ((*entry_p & 0x80) == 0);

        if (is_present) {
            if (level_matches_granularity && !can_descend) {
                // we've hit the right page, and we have no more work to do
                set_flags(entry_p, mem_flags, false);
                break;
            } else if (!level_matches_granularity && can_descend) {
                // we're not done. set flags, and descend
                set_flags(entry_p, mem_flags, true);
                cur_level -= 1;
                page_table = (page_table_t *)ADDR_SIGN_EXTEND(
                    (*entry_p & 0x000FFFFF'FFFFF000) + hhdm_off);
            } else if (level_matches_granularity && can_descend) {
                // we can clear the subdivision, set flags, and be done!
                // TODO
                printf("TODO: add_to_hhdm un-bifurcate present page\r\n");
                hcf(0, CURRENT());
            } else { // level doesn't match, but we can't descend...
                printf("entered unreachable code (add_to_hhdm level never "
                       "matched granularity before hitting the bottom of the "
                       "page table)\r\n");
                hcf(ERR_GENERIC, CURRENT());
            }
        } else { // not present
            uint64_t masked_phys_addr = phys_addr & 0x000FFFFF'FFFFFFFF &
                                        (UINT64_MAX << (cur_level * 9 + 3));
            if (level_matches_granularity) {
                // we need to make this page, and no more.
                *entry_p = masked_phys_addr;
                set_flags(entry_p, mem_flags, false);
                if (cur_level != 1) {
                    *entry_p |= 0x80;
                }
                *entry_p |= 1; // and mark present, and we're done!
                break;
            } else {
                // we need to go deeper
                page_table_t *new_table = aligned_alloc(0x1000, sizeof(page_table_t));
                memset(new_table, 0, sizeof(page_table_t));
                *entry_p = (uint64_t)new_table - hhdm_off;
                set_flags(entry_p, mem_flags, true);
                *entry_p |= 1; // mark present, and descend
            }
        }
    }
    return (void *)virt_addr;
}

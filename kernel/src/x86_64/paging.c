#include "auxv.h"
#include <memory.h>
#include <paging.h>
#include <stdint.h>
#include <stdio.h>

static void clone_page_table_level(page_table_t *orig_table,
                                   page_table_t *new_table, int level) {
    for (int idx = 0; idx < 512; idx++) {
        uint64_t entry = orig_table->entries[idx];
        new_table->entries[idx] = entry;
        if ((entry & 1) && (level > 1)) {
            page_table_t *orig_lower_table =
                (page_table_t *)((entry & 0x7FFFFFFF'FFFFF000) +
                                 getauxval(AT_KXINIX_HHDM_OFFSET).a_val);
            page_table_t *new_lower_table =
                aligned_alloc(0x1000, sizeof(page_table_t));
            clone_page_table_level(orig_lower_table, new_lower_table,
                                   level - 1);
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
        (page_table_t *)((cr3 & ~0xFFF) +
                         getauxval(AT_KXINIX_HHDM_OFFSET).a_val);
    page_table_t *new_pml4t = aligned_alloc(0x1000, sizeof(page_table_t));
    clone_page_table_level(orig_pml4t, new_pml4t, 4);

    cr3 = ((uint64_t)(new_pml4t) & ~0xFFF) | (cr3 & 0xFFF);
    printf("New CR3 = %#.16llX\r\n", cr3);
    __asm__("mov %0, %%cr3" : : "r"(cr3) : "memory");
}

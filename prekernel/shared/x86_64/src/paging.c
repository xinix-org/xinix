#include "cpuid.h"
#include <vmap.h>

extern uintptr_t hhdm_offset;

struct page_table {
    uint64_t entries[512];
};

void *vprotect(void *base, size_t page_count, enum valloc_flags prot_flags) {
    uint64_t cr3;
    __asm__("mov %%cr3, %0" : "=r"(cr3));
    bool nx = is_x86_feature_detected(nx) && !(prot_flags & PROT_EXEC);
    struct page_table *pml4t =
        (struct page_table *)((cr3 & ~0xFFFLL) + hhdm_offset);
    uint64_t base_address = ((uint64_t)base) & 0xFFFFFFFF'FFFFF000;
    uint64_t last_address =
        (base_address + page_count * 0x1000 - 1) & 0xFFFFFFFF'FFFFF000;

    for (uint64_t addr_pml4 = base_address; addr_pml4 < last_address;
         addr_pml4 = (addr_pml4 + 0x80'00000000) & 0xFFFFFF80'00000000) {
        uint64_t pml4e = pml4t->entries[(addr_pml4 >> 39) & 0x1FF];
        uint64_t pdpt_addr = pml4e & 0x000FFFFF'FFFFF000;
        struct page_table *pdpt =
            (struct page_table *)(pdpt_addr + hhdm_offset);

        for (uint64_t addr_pdp = addr_pml4;
             addr_pdp < last_address && addr_pdp < (addr_pml4 + 0x8000000000);
             addr_pdp = (addr_pdp + 0x40000000) & 0xFFFFFFFF'C0000000) {
            uint64_t *pdpe_p = &pdpt->entries[(addr_pdp >> 30) & 0x1FF];
            *pdpe_p |= 0x20; // set writable at the top level
            if ((*pdpe_p & 0x80) != 0) {
                // TODO: properly subdivide the table. Until then, we make the
                // area maximally-permissive, just in case.
                *pdpe_p &= ~0x80000000'00000000; // clear NX
                continue;
            }
            uint64_t pdt_addr = *pdpe_p & 0x000FFFFF'FFFFF000;
            struct page_table *pdt =
                (struct page_table *)(pdt_addr + hhdm_offset);

            for (uint64_t addr_pd = addr_pdp;
                 addr_pd < last_address && addr_pd < (addr_pdp + 0x40000000);
                 addr_pd = (addr_pd + 0x200000) & 0xFFFFFFFF'FFE00000) {
                uint64_t *pde_p = &pdt->entries[(addr_pd >> 21) & 0x1FF];
                *pde_p |= 0x20; // set writable at the top level
                if ((*pde_p & 0x80) != 0) {
                    // TODO: properly subdivide the table. Until then, we make
                    // the area maximally-permissive, just in case.
                    *pde_p &= ~0x80000000'00000000; // clear NX
                    continue;
                }
                uint64_t pt_addr = *pde_p & 0x000FFFFF'FFFFF000;
                struct page_table *pt =
                    (struct page_table *)(pt_addr + hhdm_offset);

                for (uint64_t addr_pt = addr_pd;
                     addr_pt < last_address && addr_pt < (addr_pdp + 0x200000);
                     addr_pt = (addr_pt + 0x1000) & 0xFFFFFFFF'FFFFF000) {
                    uint64_t *pte_p = &pt->entries[(addr_pt >> 12) & 0x1FF];
                    // TODO: handle the read bit. maybe.
                    if (prot_flags & PROT_WRITE) {
                        *pte_p |= 0x20; // set R/W
                    } else {
                        *pte_p &= ~0x20ull; // clear R/W
                    }
                    if (nx) { // calculated earlier because of feature check
                        *pte_p |= 0x80000000'00000000; // set NX
                    } else {
                        *pte_p &= ~0x80000000'00000000; // clear NX
                    }
                    __asm__ volatile("invlpg (%0)" : : "r"(addr_pt));
                }
            }
        }
    }

    return base;
}

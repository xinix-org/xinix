#pragma once

#include <stdint.h>

typedef struct [[gnu::aligned(0x1000)]] page_table {
    uint64_t entries[512];
} page_table_t;

typedef enum page_granularity : uint8_t {
    PAGE_GRANULARITY_4KB = 0,
    PAGE_GRANULARITY_2MB = 1,
    PAGE_GRANULARITY_1GB = 2
} page_granularity_t;

typedef enum mem_flags : uint32_t {
    PROT_NONE = 0,
    PROT_READ = 0x01,
    PROT_WRITE = 0x02,
    PROT_EXEC = 0x04,

    FLAG_UNINIT_MEM = 0x20,
} mem_flags_t;

extern page_table_t *kernel_pml4t;

// returns a new PML4T and switches to it
page_table_t *clone_page_table(void);

// returns virtual address in HHDM
void *add_to_hhdm(page_table_t *page_table, uint64_t phys_addr,
                  page_granularity_t granularity, mem_flags_t mem_flags);

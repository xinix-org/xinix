#pragma once

#include <stdint.h>

typedef struct [[gnu::aligned(0x1000)]] page_table {
    uint64_t entries[512];
} page_table_t;

// returns a new PML4T and switches to it
page_table_t *clone_page_table(void);

#include <stdint.h>

typedef struct [[gnu::aligned(0x1000)]] pt {
    uint64_t entries[512];
} pt_t;

typedef struct [[gnu::aligned(0x1000)]] pdt {
    uint64_t entries[512];
} pdt_t;

typedef struct [[gnu::aligned(0x1000)]] pdpt {
    uint64_t entries[512];
} pdpt_t;

typedef struct [[gnu::aligned(0x1000)]] pml4t {
    uint64_t entries[512];
} pml4t_t;

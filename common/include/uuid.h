#pragma once

#include <stdint.h>

typedef struct {
    uint64_t uuid_lo;
    uint64_t uuid_hi;
} uuid;

#define UUID_DEF(_hi, _mid1, _mid2, _mid3, _lo) \
    ((uuid){.uuid_lo = (UINT64_C(_mid3) << 48) | UINT64_C(_lo), .uuid_hi = (UINT64_C(_hi) << 32) | (UINT64_C(_mid1) << 16) | UINT64_C(_mid2)})

    
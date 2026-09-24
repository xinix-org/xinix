#pragma once

#include <stdint.h>

typedef struct {
    _Alignas(16) uint64_t uuid_lo;
    uint64_t uuid_hi;
} uuid;

#define UUID_DEF(_hi, _mid1, _mid2, _mid3, _lo)                                \
    ((uuid){.uuid_lo = ((uint64_t)(_mid3) << 48) | (uint64_t)(_lo),                \
            .uuid_hi = ((uint64_t)(_hi) << 32) | ((uint64_t)(_mid1) << 16) |       \
                       (uint64_t)(_mid2)})

#define NIL_UUID ((uuid){})
#define FULL_UUID ((uuid){.uuid_lo = ~UINT64_C(0), .uuid_hi = ~UINT64_C(0)})

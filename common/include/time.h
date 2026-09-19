#pragma once

#include <stdint.h>

typedef struct duration {
    int64_t time_seconds;
    uint32_t time_nanos;
} duration_t;

static inline duration_t duration_add(duration_t d1, duration_t d2) {
    d1.time_seconds += d2.time_seconds;
    d1.time_nanos += d2.time_nanos;
    if(d1.time_nanos > 1'000'000'000) {
        d1.time_seconds ++;
        d1.time_nanos -= 1'000'000'000;
    }

    return d1;
}

static inline duration_t duration_sub(duration_t d1, duration_t d2) {
    d1.time_seconds -= d2.time_seconds;
    d1.time_nanos -= d2.time_nanos;
    if(d1.time_nanos > 1'000'000'000) {
        d1.time_seconds --;
        d1.time_nanos += 1'000'000'000;
    }

    return d1;
}
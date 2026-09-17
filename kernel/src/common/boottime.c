#include "time.h"
#include <clock.h>
#include <stdatomic.h>

static _Atomic(uint64_t) boottime; 

void isr_increment_boottime(unsigned val) {
    atomic_fetch_add_explicit(&boottime, (uint64_t)val, memory_order_relaxed);
}

uint64_t read_boottime_micros() {
    return atomic_load_explicit(&boottime, memory_order_relaxed);
}
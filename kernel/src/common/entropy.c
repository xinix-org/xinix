#pragma once

#include "context.h"
#include "event.h"
#include <stdatomic.h>
#include <stdint.h>
#include <cpuid.h>
#include <stdio.h>
#include <string.h>
#include <stdbit.h>
#include <random.h>

extern int __rand_slow_get_entropy_backing(uint8_t _output[static restrict 16]);

uint32_t __rand_get_event_time(void);

static _Atomic(uint64_t) entropy_buffer[1024] = {};

static _Atomic(size_t) read_head;
static _Atomic(size_t) write_head;

void push_event(uint32_t r_event) {
    uint64_t total_event = (((uint64_t)r_event) << 32) | __rand_get_event_time();

    size_t write_pos = atomic_load_explicit(&write_head, memory_order_acquire);

    size_t read_pos = atomic_load_explicit(&read_head, memory_order_relaxed);
    if(write_pos == (read_pos - 1))
        return;

    atomic_store_explicit(&entropy_buffer[write_pos & 1023], total_event, memory_order_relaxed);

    atomic_compare_exchange_strong_explicit(&write_head, &write_pos, write_pos + 1, memory_order_release, memory_order_relaxed);
}

static uint64_t poll_ebuf(void) {
    size_t read_pos = atomic_load_explicit(&read_head, memory_order_relaxed);
    uint64_t val;
    do {
        size_t write_pos = atomic_load_explicit(&write_head, memory_order_acquire);
        if(read_pos == write_pos)
            return 0;
        val = atomic_load_explicit(&entropy_buffer[read_pos & 1023], memory_order_relaxed);
    } while(atomic_compare_exchange_strong_explicit(&read_head, &read_pos, read_pos + 1, memory_order_release, memory_order_relaxed));

    return val;
}

int rand_slow_get_entropy(uint8_t  _output[static restrict 16]) {
    int val = __rand_slow_get_entropy_backing(_output);
    if(val < 0)
        return val;
    union {
        uint8_t ibuf[16];
        uint64_t ebuf[2];
    } zbuf = {};

    zbuf.ebuf[0] = poll_ebuf();
    zbuf.ebuf[1] = poll_ebuf();

    for(size_t i = 0; i < 16; i++)
        _output[i] ^= zbuf.ibuf[i];
    
    return 0;

}

void random_global_gen(uint8_t out[static restrict 16]) {
    auto ctx = getcontext();
    auto uctx = ctx->current_thread;
    random_generator* gen = uctx->urand_gen;

    if(rand_ticks_since_inject(gen) > 16){
        uint8_t buf[16];
        rand_slow_get_entropy(buf);
        rand_injest(gen, buf);
    }

    rand_poll(gen, out);
}

int random_kglobal_gen(uint8_t out[static restrict 16]) {
    auto ctx = getcontext();
    random_generator* gen = &ctx->kgen;
    if(atomic_fetch_or_explicit(&ctx->kgen_lock, 1, memory_order_acquire))
        return -1;

    if(rand_ticks_since_inject(gen) > 16){
        uint8_t buf[16];
        rand_slow_get_entropy(buf);
        rand_injest(gen, buf);
    }

    rand_poll(gen, out);

    atomic_store_explicit(&ctx->kgen_lock, 0, memory_order_release);

    return 0;
}
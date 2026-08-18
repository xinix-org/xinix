#pragma once

#include "auxfuncs.h"
#include "context.h"
#include "event.h"
#include <cpuid.h>
#include <random.h>
#include <stdatomic.h>
#include <stdbit.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

extern int __rand_slow_get_entropy_backing(uint8_t _output[static restrict 16]);

uint32_t __rand_get_event_time(void);

static _Atomic(uint64_t) entropy_buffer[1024] = {};

static _Atomic(size_t) read_head;
static _Atomic(size_t) write_head;

void push_event(uint32_t r_event) {
    uint64_t total_event =
        (((uint64_t)r_event) << 32) | __rand_get_event_time();

    size_t write_pos = atomic_load_explicit(&write_head, memory_order_acquire);

    size_t read_pos = atomic_load_explicit(&read_head, memory_order_relaxed);
    if (write_pos == (read_pos - 1))
        return;

    atomic_store_explicit(&entropy_buffer[write_pos & 1023], total_event,
                          memory_order_relaxed);

    atomic_compare_exchange_strong_explicit(&write_head, &write_pos,
                                            write_pos + 1, memory_order_release,
                                            memory_order_relaxed);
}

static uint64_t poll_ebuf(void) {
    size_t read_pos = atomic_load_explicit(&read_head, memory_order_relaxed);
    uint64_t val;
    do {
        size_t write_pos =
            atomic_load_explicit(&write_head, memory_order_acquire);
        if (read_pos == write_pos)
            return 0;
        val = atomic_load_explicit(&entropy_buffer[read_pos & 1023],
                                   memory_order_relaxed);
    } while (atomic_compare_exchange_strong_explicit(
        &read_head, &read_pos, read_pos + 1, memory_order_release,
        memory_order_relaxed));

    return val;
}

int rand_slow_get_entropy(uint8_t _output[static restrict 16]) {
    int val = __rand_slow_get_entropy_backing(_output);
    if (val < 0)
        return val;
    union {
        uint8_t ibuf[16];
        uint64_t ebuf[2];
    } zbuf = {};

    zbuf.ebuf[0] = poll_ebuf();
    zbuf.ebuf[1] = poll_ebuf();

    for (size_t i = 0; i < 16; i++)
        _output[i] ^= zbuf.ibuf[i];

    return 0;
}

#define USER_TICKS_SINCE_INGEST 128
#define KERNEL_TICKS_SINCE_INGEST 16

int random_global_gen(uint8_t out[static restrict 16]) {
    auto ctx = getcontext();
    auto uctx = ctx->current_thread;
    random_generator *gen = uctx->urand_gen;

    if (rand_ticks_since_ingest(gen) >= USER_TICKS_SINCE_INGEST) {
        uint8_t buf[16];
        rand_slow_get_entropy(buf);
        rand_ingest(gen, buf);
    }

    rand_poll(gen, out);

    return 0;
}

int random_kglobal_gen(uint8_t out[static restrict 16]) {
    auto ctx = getcontext();
    random_generator *gen = &ctx->kgen;
    if (atomic_fetch_or_explicit(&ctx->kgen_lock, 1, memory_order_acquire))
        return -1;

    if (rand_ticks_since_ingest(gen) >= KERNEL_TICKS_SINCE_INGEST) {
        uint8_t buf[16];
        rand_slow_get_entropy(buf);
        rand_ingest(gen, buf);
    }

    rand_poll(gen, out);

    atomic_store_explicit(&ctx->kgen_lock, 0, memory_order_release);

    return 0;
}

void random_global_ingest(const uint8_t out[static restrict 16]) {
    auto ctx = getcontext();
    auto uctx = ctx->current_thread;
    random_generator *gen = uctx->urand_gen;
    if (rand_ticks_since_ingest(gen) >= (USER_TICKS_SINCE_INGEST / 2)) {
        uint8_t buf[16];
        rand_slow_get_entropy(buf);
        rand_ingest(gen, buf);
    }

    rand_ingest(gen, out);
}

void random_kglobal_ingest(const uint8_t out[static restrict 16]) {
    auto ctx = getcontext();
    random_generator *gen = &ctx->kgen;
    while (atomic_fetch_or_explicit(&ctx->kgen_lock, 1, memory_order_acquire))
        spin_loop_hint();

    if (rand_ticks_since_ingest(gen) >= (KERNEL_TICKS_SINCE_INGEST / 2)) {
        uint8_t buf[16];
        rand_slow_get_entropy(buf);
        rand_ingest(gen, buf);
    }

    rand_ingest(gen, out);

    atomic_store_explicit(&ctx->kgen_lock, 0, memory_order_release);
}

struct rand_dev_data {
    int (*gen)(uint8_t[static restrict 16]);
    void (*ingest)(const uint8_t[static restrict 16]);
};

static struct rand_dev_data global_data = {
    .gen = random_global_gen,
    .ingest = random_global_ingest,
};

static struct rand_dev_data kglobal_data = {
    .gen = random_kglobal_gen,
    .ingest = random_kglobal_ingest,
};

static size_t rd_write(void *data, size_t sz, const void *buf) {
    const uint8_t *rdata = buf;
    struct rand_dev_data *desc = (struct rand_dev_data *)data;
    size_t total_written = sz;
    while (sz >= 16) {
        (desc->ingest)(rdata);
        rdata += 16;
        sz -= 16;
    }

    uint8_t rest[16];
    memcpy(rest, buf, sz);
    rest[sz + 1] = 0xF8;
    rest[15] = 0x01;
    (desc->ingest)(buf);

    return total_written;
}

static size_t rd_read(void *data, size_t sz, void *buf) {
    uint8_t *rdata = buf;
    struct rand_dev_data *desc = (struct rand_dev_data *)data;
    size_t total_read = 0;
    while (sz >= 16) {
        if ((desc->gen)(rdata) < 0)
            return total_read;
        rdata += 16;
        sz -= 16;
        total_read += 16;
    }

    if (sz > 0) {
        uint8_t rest[16];
        if ((desc->gen)(rest) < 0)
            return total_read;
        memcpy(rdata, rest, sz);
        total_read += 16;
    }
    return total_read;
}

static FILE impl_rand_dev = {
    .data = &global_data, .write = rd_write, .read = rd_read};
static FILE impl_krand_dev = {
    .data = &kglobal_data, .write = rd_write, .read = rd_read};

FILE *krand_dev = &impl_krand_dev;
FILE *rand_dev = &impl_rand_dev;

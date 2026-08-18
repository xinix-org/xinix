#pragma once

#include <random.h>

#include <stdatomic.h>
#include <usercontext.h>

typedef struct user_context ucontext_t;

typedef struct kernel_context {
    _Alignas(256) struct kernel_context *self;
    size_t total_context_size;
    struct user_context *current_thread;
    _Atomic(size_t) kgen_lock;
    random_generator kgen;
} kcontext_t;

kcontext_t *getcontext(void);

/// Saves registers not saved by default on a context switch to `uctx`
void save_full_ucontext(ucontext_t *uctx);
/// Saves register state needed for handling a debugging interrupt to `uctx`
void save_debug_ucontext(ucontext_t *uctx);
/// Loads registers not saved by default on a context switch from `uctx`
void load_full_ucontext(ucontext_t *uctx);

int random_global_gen(uint8_t _out[static restrict 16]);
int random_kglobal_gen(uint8_t _out[static restrict 16]);

void random_global_ingest(const uint8_t _buf[static restrict 16]);
void random_kglobal_ingest(const uint8_t _buf[static restrict 16]);

void print_ucontext(const ucontext_t *uctx);

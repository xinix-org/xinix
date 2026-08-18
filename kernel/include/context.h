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

void save_full_ucontext(ucontext_t *uctx);
void load_full_ucontext(ucontext_t *uctx);

void random_global_gen(uint8_t _out[static restrict 16]);
int random_kglobal_gen(uint8_t _out[static restrict 16]);
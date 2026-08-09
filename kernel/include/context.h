#pragma once

#include <random.h>

#include <usercontext.h>

typedef struct user_context ucontext_t;

typedef struct kernel_context {
    struct kernel_context *self;
    size_t total_context_size;
    struct user_context *current_thread;
    random_generator kgen;
} kcontext_t;

kcontext_t *getcontext(void);

void save_full_ucontext(ucontext_t *uctx);
void load_full_ucontext(ucontext_t *uctx);

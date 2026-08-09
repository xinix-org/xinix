#pragma once

#include <random.h>

#include <usercontext.h>

typedef struct user_context ucontext_t;

typedef struct kernel_context {
    struct kernel_context *self;
    size_t total_context_size;
    random_generator kgen;
    struct user_context *current_thread;
} kcontext_t;

kcontext_t *getcontext(void);

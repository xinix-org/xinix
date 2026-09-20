#pragma once

#include "acpi.h"
#include "paging.h"
#include "sysresult.h"
#include <random.h>
#include <time.h>

#include <stdatomic.h>

struct thread;

#include <usercontext.h>


typedef struct user_context ucontext_t;

typedef struct kernel_context {
    _Alignas(256) struct kernel_context *self;
    size_t total_context_size;
    ucontext_t *current_thread;
    // DO NOT UNDER ANY CIRCUMSTANCES ADD ANY FIELDS ABOVE THIS LINE!!!
    // YOU WILL BREAK INTERUPT HANDLING CODE
    bool is_root_context;
    _Atomic(size_t) kgen_lock;
    random_generator kgen;
    duration_t last_timer_tsc;
    volatile lapic_t *lapic; // TODO: thread-local
} kcontext_t;

kcontext_t *getcontext(void);

/// Saves registers not saved by default on a context switch to `uctx`
void save_full_ucontext(ucontext_t *_uctx);
/// Loads registers not saved by default on a context switch from `uctx`
void load_full_ucontext(ucontext_t *_uctx);

int random_global_gen(uint8_t _out[static restrict 16]);
int random_kglobal_gen(uint8_t _out[static restrict 16]);

void random_global_ingest(const uint8_t _buf[static restrict 16]);
void random_kglobal_ingest(const uint8_t _buf[static restrict 16]);

void print_ucontext(const ucontext_t *_uctx);


/// Creates a new ucontext_t for a specified process
sysresult2_t create_context(struct thread* _th, paddr_t _ptable);


kcontext_t *lock_context(void);
void unlock_context(void);
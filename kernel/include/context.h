#pragma once

#include "acpi.h"
#include "paging.h"
#include "sysresult.h"
#include <random.h>
#include <stddef.h>
#include <time.h>

#include <stdatomic.h>

struct thread;

#include <usercontext.h>

typedef struct user_context ucontext_t;

typedef struct kernel_context {
    _Alignas(256) struct kernel_context *self;
    size_t total_context_size;
    ucontext_t *current_thread;
    ucontext_t *current_uthread;
    ucontext_t *current_systhread;
    // DO NOT UNDER ANY CIRCUMSTANCES ADD ANY FIELDS ABOVE THIS LINE!!!
    // YOU WILL BREAK INTERUPT HANDLING CODE
    // If you need to add a field above, ensure the static asserts continue to pass, and add new ones as needed.
    // Only add fields that are used in assembly above.
    bool is_root_context;
    _Atomic(size_t) kgen_lock;
    random_generator kgen;
    duration_t last_timer_tsc;
    volatile lapic_t *lapic; // TODO: thread-local
} kcontext_t;

// Do not break under any circumstances. context code relies on this reflexive relation
static_assert(offsetof(kcontext_t, self) == 0);

// If necessarily broken, adjust `idt.s` and `syscall.s`
static_assert(offsetof(kcontext_t, current_thread) == 2 * sizeof(void*));
static_assert(offsetof(kcontext_t, current_uthread) == 3 * sizeof(void*));
static_assert(offsetof(kcontext_t, current_systhread) == 4 * sizeof(void*));

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
sysresult2_t create_context(struct thread *_th, paddr_t _ptable);

kcontext_t *lock_context(void);
void unlock_context(void);

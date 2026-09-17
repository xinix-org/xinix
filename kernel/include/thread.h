#pragma once

#include "context.h"
#include "time.h"
#include <uuid.h>

struct process {
    uuid proc_owner;
    struct thread** proc_threads;
    size_t proc_nthreads;
    size_t proc_cthreads;
};

struct thread {
    _Atomic(kcontext_t*) thrd_resident;
    uuid thrd_owner;
    struct process* thd_proc;
    ucontext_t* thrd_uctx;
    bool thrd_is_kernel;
    duration_t thrd_last_tsc;
    duration_t thrd_cmono;
};
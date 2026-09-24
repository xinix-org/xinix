#pragma once

#include "context.h"
#include "time.h"
#include <stddef.h>
#include <stdint.h>
#include <uuid.h>

struct process {
    uint64_t proc_uid;
    uint64_t proc_gid;
    struct thread **proc_threads;
    size_t proc_nthreads;
    size_t proc_cthreads;
};

struct thread {
    _Atomic(kcontext_t *) thrd_resident;
    uint64_t thrd_uid;
    uint64_t thrd_gid;
    struct process *thd_proc;
    ucontext_t *thrd_uctx;
    ucontext_t *thrd_sysctx;
    bool thrd_is_kernel;
    duration_t thrd_last_tsc;
    duration_t thrd_cmono;
};

#define UID_KERNEL (~UINT64_C(0))

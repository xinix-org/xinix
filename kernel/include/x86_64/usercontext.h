#pragma once

#include <random.h>
#include <stdint.h>

#define FXSAVE_SIZE 512

struct user_context {
    void *gregs[16];
    void *rip;
    uint64_t rflags;
    /// Legacy Segment entries
    /// Order: [es, cs, ds, ss, fs, gs, tss, ldt]
    uint16_t sregs[8];
    void *tdata;
    void *fsgsbase[2];
    uint64_t cr3;
    void *dregs[6];
    uint64_t thread_flags[4];
    random_generator *urand_gen;
    void *_pad[21];

    // Keep these fields near `fxsave`
    uint64_t xcr0_allowed;
    /// If 0, no xsave/fxsave data stored at all
    /// If `FXSAVE_SIZE`, contains only fxsave state
    /// If `>FXSAVE_SIZE`, contains xsave state starting at `fxsave`
    uint64_t xsave_size;
    _Alignas(64) uint64_t fxsave[FXSAVE_SIZE / 8];
    uint64_t restxsave[];
};

// Santity check that we have the right amount of padding
static_assert(sizeof(struct user_context) == 1024);

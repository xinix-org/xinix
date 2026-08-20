#pragma once

#include <random.h>
#include <stdint.h>

#define FXSAVE_SIZE 512

struct streg {
    _Alignas(16) uint64_t lo;
    uint16_t hi;
    uint16_t res[3];
};

union xmmreg {
    _Alignas(16) uint64_t u64x2[2];
    uint32_t u32x4[4];
    float f32x4[4];
    double f64x2[2];
};

struct fxsave_state {
    _Alignas(16) uint16_t fcw;
    uint16_t fsw;
    uint8_t ftw;
    uint8_t _reserved5;
    uint16_t fop;
    void *fip;
    void *fdp;
    uint32_t mxcsr;
    uint32_t mxcsr_mask;
    struct streg fp[8];
    union xmmreg xmm[16];
    uint64_t reserved416[6];
    uint64_t avail[6];
};

static_assert(sizeof(struct fxsave_state) == 512);

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
    random_generator *urand_gen;
    void *dregs[6];
    uint64_t thread_flags[4];
    uint64_t cr4;
    void *_pad[20];

    // Keep these fields near `fxsave`
    uint64_t xcr0_allowed;
    /// If 0, no xsave/fxsave data stored at all
    /// If `FXSAVE_SIZE`, contains only fxsave state
    /// If `>FXSAVE_SIZE`, contains xsave state starting at `fxsave`
    uint64_t xsave_size;
    _Alignas(64) struct fxsave_state fxsave;
    _Alignas(64) uint64_t restxsave[];
};

// Santity check that we have the right amount of padding
static_assert(sizeof(struct user_context) == 1024);

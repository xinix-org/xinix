#pragma once

#include <stdint.h>

#define FXSAVE_SIZE 64

struct user_context {
    void* gregs[16];
    void* fsgsbase[2];
    /// Legacy Segment entries
    /// Order: [es, cs, ds, ss, fs, gs, tss, ldt]
    uint16_t sregs[8];
    void* tdata;
    uint64_t cr3;
    void* dregs[6];
    uint64_t thread_flags[4];
    void* _pad[31];

    /// If 0, no xsave/fxsave data stored at all
    /// If `FXSAVE_SIZE`, contains only fxsave state
    /// If `>FXSAVE_SIZE`, contains xsave state starting at `fxsave`
    uint64_t xsave_size;
    _Alignas(64) uint64_t fxsave[FXSAVE_SIZE];
    uint64_t restxsave[];
};

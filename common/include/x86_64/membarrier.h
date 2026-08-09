#pragma once

#include <cpuid.h>

/// Forms a memory barrier with the compiler
/// Note that this does not synchronize accesses
static inline void compiler_barrier(void) { __asm__ volatile("" ::: "memory"); }

/// Forms a memory barrier with the cpu
/// Note that this does not synchronize accesses
static inline void hard_barrier(void) {
    if is_x86_feature_detected (sse) {
        __asm__ volatile("mfence" ::: "memory");
    } else {
        __asm__ volatile("cpuid" ::: "memory", "eax", "edx", "ecx", "ebx");
    }
}

#pragma once

#include <stdint.h>

static inline uint8_t inb(uint16_t port) {
    uint8_t data;
    __asm__ volatile("inb %w1, %b0" : "=a"(data) : "Nd"(port) : "memory");
    return data;
}

static inline uint16_t inw(uint16_t port) {
    uint16_t data;
    __asm__ volatile("inw %w1, %w0" : "=a"(data) : "Nd"(port) : "memory");
    return data;
}

static inline uint32_t inl(uint16_t port) {
    uint32_t data;
    __asm__ volatile("inl %w1, %l0" : "=a"(data) : "Nd"(port) : "memory");
    return data;
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

static inline void outw(uint16_t port, uint16_t val) {
    __asm__ volatile("outw %w0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

static inline void outl(uint16_t port, uint32_t val) {
    __asm__ volatile("outl %l0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

static inline unsigned long read_cr4(void) {
    unsigned long ret;
    __asm__ volatile("mov %0, %%cr4" : "=r"(ret));

    return ret;
}

#define CR4_OSFXSR (1UL << 9)
#define CR4_OSXSAVE (1UL << 18)

static inline void write_cr4(unsigned long val) {
    __asm__ volatile("mov %%cr4, %0" : : "r"(val) : "memory");
}


static inline uint64_t read_msr(uint32_t msr) {
    unsigned long retA, retD;

    __asm__ volatile("rdmsr" : "=a"(retA), "=d"(retD) : "c"(msr));

    return ((uint64_t)retA) | ((uint64_t)retD) << 32;
}

#if UINTPTR_WIDTH == 64
#define READ_MSR_PTR_EXTRA "\r\n\tshlq 32, %%rdx\r\n\tor %%rdx, %%rax"
#define WRITE_MSR_PTR_EXTRA "mov %%rax, %%rdx\r\n\tshrq 32, %%rdx\r\n\t"
#else 
#define READ_MSR_PTR_EXTRA ""
#define WRITE_MSR_PTR_EXTRA "xor %%eax, %%edx\r\n\t"
#endif

static inline void* read_msr_pointer(uint32_t msr) {
    void* ret;

    __asm__ volatile("rdmsr" READ_MSR_PTR_EXTRA : "=a"(ret) : "c" (msr) : "edx", "cc");

    return ret;
}

static inline void write_msr(uint32_t msr, uint64_t value) {
    unsigned long valA = value & 0xFFFF'FFFF, valD = (value) >> 32;

    __asm__ volatile("wrmsr" : : "a" (valA), "d" (valD), "c" (msr) : "memory");
}

static inline void write_msr_pointer(uint32_t msr, void* value) {
    __asm__ volatile(WRITE_MSR_PTR_EXTRA "wrmsr" :: "a" (value), "c"(msr) : "memory", "cc", "edx");
}

#undef READ_MSR_PTR_EXTRA
#undef WRITE_MSR_PTR_EXTRA

#define IA32_STAR UINT32_C(0xC0000081)
#define IA32_LSTAR UINT32_C(0xC0000082)
#define IA32_CSTAR UINT32_C(0xC0000083)
#define IA32_FMASK UINT32_C(0xC0000084)
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
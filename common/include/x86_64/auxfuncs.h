#pragma once
#include <bits/feat_test.h>
#include <location.h>
#include <sysresult.h>

#define DEBUG_MAGIC 0xDEADBEEFCAFEBABEULL
#define DEBUG_MAGIC2 0x6c44198c4a475817ULL

[[noreturn, gnu::noinline, gnu::cold]]
static void hcf(sysresult_t _err, source_location_t * _loc, const char* _msg) {

#if __has_builtin(__builtin_return_address)
    void *_ra = __builtin_return_address(0);
#else
    void *_ra = nullptr;
#endif

    __asm__ volatile("mov %0, %%r15\r\nmov %0, %%r12\r\nint3" ::"r"(DEBUG_MAGIC2), "r"(_msg), "D"(_err),
                     "S"(_loc->src_file), "d"(_loc->src_function),
                     "c"(_loc->src_line), "a"(_ra), "b"(DEBUG_MAGIC)
                     : "r15", "r12"); // Trigger a debug trap
    for (;;) {
        __asm__ volatile("hlt");
    }
}


static inline void spin_loop_hint(void) {
#if __has_builtin(__builtin_ia32_pause)
    __builtin_ia32_pause();
#else
    __asm__ volatile("pause");
#endif
}

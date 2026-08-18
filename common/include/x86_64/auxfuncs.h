#pragma once
#include <bits/feat_test.h>
#include <location.h>
#include <sysresult.h>

#define DEBUG_MAGIC 0xDEADBEEFCAFEBABEUL
#define DEBUG_MAGIC2 0x6c44198c4a475817UL

[[noreturn, gnu::noinline]]
static void hcf(sysresult_t err, source_location_t *loc) {

#if __has_builtin(__builtin_return_address)
    void *ra = __builtin_return_address(0);
#else
    void *ra = nullptr;
#endif

    __asm__ volatile("mov %0, %%r15\r\nint3" ::"r"(DEBUG_MAGIC2), "D"(err),
                     "S"(loc->src_file), "d"(loc->src_function),
                     "c"(loc->src_line), "a"(ra), "b"(DEBUG_MAGIC)
                     : "r15"); // Trigger a debug trap
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

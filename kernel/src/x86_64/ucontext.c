#include "cpu.h"
#include "gdt.h"
#include "paging.h"
#include "random.h"
#include "sysresult.h"
#include "usercontext.h"
#include <context.h>
#include <memory.h>
#include <cpuid.h>
#include <string.h>
#include <thread.h>


sysresult2_t create_context(struct thread* th, paddr_t ptable) {
    uint8_t seed[16];

    if(random_kglobal_gen(seed) < 0)
        return SYSRESULT2_ERROR(ERR_GENERIC);
    
    bool thread_is_kernel = th->thrd_is_kernel;
    uint16_t default_data_seg = thread_is_kernel ? GDT_KDATA64 : GDT_UDATA64;
    uint16_t default_code_seg = thread_is_kernel ? GDT_KCODE64 : (GDT_UCODE64 | 3);
    auto cr4 = read_cr4();

    size_t xsave_size; // enabling xsave is per process
    if(cr4 & CR4_OSFXSR) {
        xsave_size = FXSAVE_SIZE;
    } else {
        xsave_size = 0;
    }
    ucontext_t* ctx = aligned_alloc(alignof(ucontext_t),sizeof(ucontext_t) + xsave_size);
    if(!ctx)
        return SYSRESULT2_ERROR(ERR_INSUFFICIENT_MEMORY);
    memset(ctx, 0, sizeof(ucontext_t) + xsave_size);

    ctx->xsave_size = xsave_size;
    ctx->xcr0_allowed = ((uint64_t)x86_feature_array[32]) | ((uint64_t)x86_feature_array[33]) << 32;
    ctx->fsgsbase[0] = nullptr;
    if(thread_is_kernel)
        ctx->fsgsbase[1] = getcontext();
    else
        ctx->fsgsbase[1] = nullptr;
    ctx->sregs[0] = default_data_seg;
    ctx->sregs[1] = default_code_seg;
    ctx->sregs[2] = default_data_seg;
    ctx->sregs[3] = default_data_seg;
    ctx->sregs[4] = 0;
    ctx->sregs[5] = 0;

    ctx->cr4 = cr4;
    ctx->tdata = th;
    ctx->cr3 = ptable;

    ctx->urand_gen = aligned_alloc(alignof(random_generator), sizeof(random_generator));

    if(!ctx->urand_gen) {
        free(ctx);
        return SYSRESULT2_ERROR(ERR_INSUFFICIENT_MEMORY);
    }

    memset(ctx->urand_gen, 0, sizeof(random_generator));
    rand_ingest(ctx->urand_gen, seed);

    th->thrd_uctx = ctx;

    return SYSRESULT2_OK(ctx);
}
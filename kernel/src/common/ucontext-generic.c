#include "auxfuncs.h"
#include <context.h>
#include <stdatomic.h>
#include <thread.h>

kcontext_t *lock_context(void) {
    kcontext_t *ctx = getcontext();

    atomic_exchange_explicit(&ctx->current_thread->tdata->thrd_resident, ctx,
                             memory_order_acquire);
    while (ctx != getcontext())
        spin_loop_hint(); // make this a yield later

    return ctx;
}

void unlock_context() {
    kcontext_t *ctx = getcontext();
    atomic_store_explicit(&ctx->current_thread->tdata->thrd_resident, nullptr,
                          memory_order_release);
}

#pragma once

#include "sysresult.h"
#include <stdint.h>
#include <usercontext.h>

enum xsave_component {
    xsave_x87 = 0,
    xsave_xmm = 1,
    xsave_avx = 2,
    xsave_mpxbnd = 3,
    xsave_mpxcs = 4,
    xsave_avx512k = 5,
    xsave_avx512zmmup = 6,
    xsave_avx512zmmx = 7,
    xsave_pt = 8,
    xsave_pkru = 9,
    xsave_pasid = 10,
    xsave_cetu = 11,
    xsave_cets = 12,
    xsave_hdc = 13,
    xsave_uint = 14,
    xsave_lbr = 15,
    xsave_hwp = 16,
    xsave_amxcfg = 17,
    xsave_amxtile = 18,
    xsave_apxegpr = 19,

    xsave_lwp = 62,
};

typedef struct fxsave_state xsave_x87_t;
typedef struct fxsave_state xsave_xmm_t;

typedef struct xsave_mpxbnd xsave_mpxbd_t;
typedef struct xsave_mpxcs xsave_mpxcs_t;
typedef struct xsave_pt xsave_pt_t;
typedef struct xsave_pkru xsave_pkru_t;
typedef struct xsave_pasid xsave_pasid_t;
typedef struct xsave_cetu xsave_cetu_t;
typedef struct xsave_cets xsave_cets_t;
typedef struct xsave_hdc xsave_hdc_t;
typedef struct xsave_uintr xsave_uintr_t;
typedef struct xsave_lbr xsave_lbr_t;
typedef struct xsave_hwp xsave_hwp_t;
typedef struct xsave_lwp xsave_lwp_t;

struct xsave_header {
    uint64_t xstate_bv;
    uint64_t xcomp_bv;
    uint64_t _reserved16[6];
};

static inline struct xsave_header* xsave_get_header(struct user_context* _uctx) {
    if (_uctx->xsave_size >= MIN_XSAVE_SIZE)
        return (struct xsave_header*)_uctx->restxsave;
    else
        return nullptr;
}

const void* xsave_get_state(const struct user_context* _uctx, enum xsave_component _comp);

typedef struct xsave_avx {
    _Alignas(64) union xmmreg ymmup[16];
} xsave_avx_t;

typedef struct xsave_avx512kreg {
    uint64_t kregs[8];
} xsave_avx512kreg_t;

struct ymmreg {
    union xmmreg pieces[2];
};

struct zmmreg {
    struct ymmreg pieces[2];
};

typedef struct xsave_avx512zmmup {
    _Alignas(64) struct ymmreg zmmup[16];
} xsave_avx512zmmup_t;

typedef struct xsave_avx512zmmx {
    _Alignas(64) struct zmmreg zmmext[16];
} xsave_avx512zmmx_t;

typedef struct xsave_amxcfg {
    _Alignas(64) uint64_t tilecfg[8];
} xsave_amxcfg_t;


struct tmmreg {
    _Alignas(64) uint64_t _value[128];
};

typedef struct xsave_amxtile {
    struct tmmreg tmm[8];
} xsave_amxtile_t;

typedef struct xsave_apxegpr {
    void* egprs[16];
} xsave_apxegpr_t;

sysresult_t xsave_read_xmm(const struct user_context* _uctx, uint8_t _vreg, struct zmmreg* _val);

#define xsave_get_state(uctx, comp)\
    _Generic((uctx), \
        struct user_context*: (comp##_t*)((xsave_get_state)(uctx, comp)),\
        const struct user_context*: (const comp##_t*)((xsave_get_state)(uctx, comp))\
    )
#include "sysresult.h"
#include <xsave.h>
#include <stdatomic.h>
#include <once.h>
#include <cpuid.h>
#include <memory.h>


static union { 
    struct {
        uint32_t size;
        uint32_t offset;
    }; 
    uint64_t alignment_spec;
    uint64_t supported_components;
} xsave_info[63];
static once_flag xsave_init;

static void init_xsave_info(void) {
    auto id = cpuid(0);
    if(id.eax < 0x0D)
        return;

    auto xsave_base = cpuid_count(0x0D, 0);
    auto xsave_ext = cpuid_count(0x0D, 1);

    xsave_info[0].supported_components = ((uint64_t)xsave_base.eax | (uint64_t)xsave_ext.ecx) | (((uint64_t)xsave_base.edx | (uint64_t)xsave_ext.edx) << 32);

    for(uint32_t i = 2; i < 63; i++) {
        if (xsave_info[0].supported_components & (1ULL << i)) {
            auto component_info = cpuid_count(0x0D, i);
            xsave_info[i].size = component_info.eax;
            xsave_info[i].offset = component_info.ebx;
            xsave_info[1].alignment_spec |= ((component_info.ecx >> 1) & 1) << i;
        }
    }
}


const void* (xsave_get_state)(const struct user_context* _uctx, enum xsave_component _comp) {
    auto xsave_hdr = xsave_get_header(_uctx);

    if (!xsave_hdr){
        // special case: If thread is using fxsave, these two state components are always saved.
        // This is not above because
        if((_comp == xsave_x87 || _comp == xsave_xmm) && _uctx->xsave_size == 512)
            return &_uctx->fxsave;
        else
            return nullptr;
    }

    // x87 is always present, but sse may not be
    if((xsave_hdr->xstate_bv | 1) & (1ULL << _comp))
        return nullptr;

    call_once(&xsave_init, init_xsave_info);

    if (_comp == xsave_x87 || _comp == xsave_xmm)
        return &_uctx->fxsave;

    if(xsave_hdr->xstate_bv & (1ULL << 63)) {
        auto offset = sizeof(struct xsave_header);
        for(uint32_t i = 2; i < _comp; i++) {
            offset += xsave_info[i].size;

            if (xsave_info[1].alignment_spec & (1ULL << i))
                offset = (offset + 63) & 63;
        }

        return ((char*)_uctx->restxsave) + offset;
    } else {
        auto offset = xsave_info[_comp].offset;

        return ((char*)(&_uctx->fxsave)) + offset;
    }
}

sysresult_t xsave_read_xmm(const struct user_context* _uctx, uint8_t _vreg, struct zmmreg *_val) {

    size_t maxvl = 0;
    if(_vreg > 16) {
        if(!(_uctx->xcr0_allowed & (1ULL << xsave_avx512zmmx)))
            return ERR_GENERIC;
        auto zmmregs = xsave_get_state(_uctx, xsave_avx512zmmx);

        if(zmmregs)
            *_val = zmmregs->zmmext[_vreg - 16];
        else
            memset(_val, 0, sizeof(struct zmmreg));

        maxvl = 64;
    } else {
        if(!(_uctx->xcr0_allowed & (1ULL << xsave_xmm)))
            return ERR_GENERIC;

        auto xmm = xsave_get_state(_uctx, xsave_xmm);

        if(xmm){
            _val->pieces[0].pieces[0] = xmm->xmm[_vreg];

            maxvl = 16;

            auto ymmup = xsave_get_state(_uctx, xsave_avx);

            if(ymmup) {
                _val->pieces[0].pieces[1] = ymmup->ymmup[_vreg];
                maxvl = 32;
                auto zmmup = xsave_get_state(_uctx, xsave_avx512zmmup);
                if(zmmup) {
                    _val->pieces[1] = zmmup->zmmup[_vreg];
                    maxvl = 64;
                }
            }
        }

        memset(((char*)_val) + maxvl, 0, sizeof(struct zmmreg)-maxvl);
    }

    return maxvl;
}
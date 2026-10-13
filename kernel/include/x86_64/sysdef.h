#pragma once 

#include <context.h>
#include <sysresult.h>

#define sysdecl __attribute__((section(".text.syscall")))

#define _do_sysparam_list(UC, P1, P2, P3, P4, P5, ...) (P1, P2, P3, P4, P5, ucontext_t (*UC))

#define sys_param_list(UC, ...) _do_sysparam_list(UC __VA_OPT__(,) __VA_ARGS__, long _ignored0, long _ignored1, long _ignored2, long _ignored3, long _ignored4)

#define sysret_void(UC) do {\
    auto _uctx = UC;\
    _uctx->gregs[0] = nullptr;\
    _uctx->gregs[2] = nullptr;\
    return _uctx;\
} while(0)

#define sysret_result(UC, res) do {\
    auto _uctx = UC;\
    _uctx->gregs[0] = (void*)res;\
    _uctx->gregs[2] = nullptr;\
    return _uctx;\
} while(0)

#define sysret_result2(UC, res, T) do {\
    auto _uctx = (UC);\
    auto _res = (res);\
    _uctx->gregs[0] = (void*)SYSRESULT2_CODE(_res);\
    if(SYSRESULT2_CODE(_res) < 0)\
        _uctx->gregs[2] = nullptr;\
    else\
        _uctx->gregs[2] = (void*)SYSRESULT2_VALUE(_res, typeof(T));\
    return _uctx;\
} while(0)
#pragma once

#include <location.h>
#include <auxfuncs.h>
#include <sysresult.h>

static inline void __do_assert(bool __val, sysresult_t __err, source_location_t * __loc, const char* __msg) {
    if(!__val) {
        hcf(__err, __loc, __msg);
    }
}

#ifndef NDEBUG
#define assert(...)\
    __do_assert((__VA_ARGS__), ERR_ASSERTION_FAILED, CURRENT(), #__VA_ARGS__)
#else
#define assert(...) (void)0
#endif
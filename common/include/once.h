#pragma once
#include <stdatomic.h>
#include <stddef.h>
#include <bits/auxfuncs.h>

typedef struct once_flag {
    _Atomic(size_t) _once_state;
} once_flag;

static inline void call_once(once_flag* _flag, void(*_call)(void)) {
    #define _once_flag_init 2
    #define _once_flag_locked 1
    if(atomic_load_explicit(&_flag->_once_state, memory_order_acquire) & _once_flag_init) return;
    while(atomic_fetch_or_explicit(&_flag->_once_state, _once_flag_locked, memory_order_acquire) & _once_flag_locked) spin_loop_hint();

    if(!(atomic_load_explicit(&_flag->_once_state, memory_order_acquire) & _once_flag_init))
        (_call)();

    atomic_store_explicit(&_flag->_once_state, _once_flag_init, memory_order_release);
}
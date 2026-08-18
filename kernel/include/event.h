#pragma once

#include <stdint.h>

enum EventKinds : uint32_t {
    EVENT_KEY = 0x01'00'0000,
    EVENT_MOUSE_BUTTON = 0x02'00'0000,
    EVENT_BUTTON_DOWN = 0x00'01'0000,
    EVENT_BUTTON_UP = 0x00'02'00000,
    EVENT_MOUSE_MOTION = 0x03'000000,
    EVENT_TIMER = 0x04'000000,
};

#define KEY(step, scode) 

/// Writes `r_event` to the 
void push_event(uint32_t r_event);

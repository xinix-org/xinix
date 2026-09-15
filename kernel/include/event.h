#pragma once

#include <stdint.h>

enum EventKinds : uint32_t {
    EVENT_KEY = 0x01'00'0000,
    EVENT_MOUSE_BUTTON = 0x02'00'0000,
    EVENT_BUTTON_DOWN = 0x00'01'0000,
    EVENT_BUTTON_UP = 0x00'02'00000,
    EVENT_MOUSE_MOTION = 0x03'000'000,
    EVENT_TIMER = 0x04'000000,
};

/// An event word for a key press
#define KEY(scode) (EVENT_KEY | (scode))

/// An event word for a mouse button press
#define MOUSE_BUTTON(step, button) (EVENT_MOUSE_BUTTON | (step) | (button))

/// An event word for a mouse motion input
#define MOUSE_MOTION(dx, dy) (EVENT_MOUSE_MOTION | ((dx) << 12) | (dy))

/// An event word for the timer interrupt
#define TIMER(dt) (EVENT_TIMER | (dt))

/// Writes `r_event` to the event buffer.
/// Event should be constructed using `KEY`, `MOUSE_BUTTON`, `MOUSE_MOTION`, or
/// TIMER`
void push_event(uint32_t r_event);

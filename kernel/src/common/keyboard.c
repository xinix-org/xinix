#include "keyboard.h"
#include <stdint.h>
#include <stdio.h>

typedef enum kbd_scan_state {
    KB_SCAN_INIT = 0,
    KB_SCAN_E0 = 1,

    NUM_KB_SCAN_STATES,

    KB_SCAN_UNKNOWN = 0xFE,
    KB_SCAN_ERROR = 0xFF,
} kbd_scan_state_t;

typedef struct kbd_scan_transition {
    kbd_scan_state_t next_state;
    kbd_scan_code_t scan_code;
} kbd_scan_transition_t;

#define KD(x) {KB_SCAN_INIT, x}
#define KU(x) {KB_SCAN_INIT, x | KEY_SCAN_RELEASE}
#define B {KB_SCAN_E0}

#define U {KB_SCAN_UNKNOWN}
#define X {KB_SCAN_ERROR}

// clang-format off
static kbd_scan_transition_t kbd_state_machine[NUM_KB_SCAN_STATES][256] = {
    // KB_SCAN_INIT
    {
        X,                     KD(KEY_SCAN_ESCAPE),    KD(KEY_SCAN_1),               KD(KEY_SCAN_2),              KD(KEY_SCAN_3),       KD(KEY_SCAN_4),            KD(KEY_SCAN_5),           KD(KEY_SCAN_6),               // 00-07
        KD(KEY_SCAN_7),        KD(KEY_SCAN_8),         KD(KEY_SCAN_9),               KD(KEY_SCAN_0),              KD(KEY_SCAN_MINUS),   KD(KEY_SCAN_EQUAL),        KD(KEY_SCAN_BACKSPACE),   KD(KEY_SCAN_TAB),             // 08-0F
        KD(KEY_SCAN_Q),        KD(KEY_SCAN_W),         KD(KEY_SCAN_E),               KD(KEY_SCAN_R),              KD(KEY_SCAN_T),       KD(KEY_SCAN_Y),            KD(KEY_SCAN_U),           KD(KEY_SCAN_I),               // 10-17
        KD(KEY_SCAN_O),        KD(KEY_SCAN_P),         KD(KEY_SCAN_BRACKET_LEFT),    KD(KEY_SCAN_BRACKET_RIGHT),  KD(KEY_SCAN_ENTER),   KD(KEY_SCAN_CONTROL_LEFT), KD(KEY_SCAN_A),           KD(KEY_SCAN_S),               // 18-1F
        KD(KEY_SCAN_D),        KD(KEY_SCAN_F),         KD(KEY_SCAN_G),               KD(KEY_SCAN_H),              KD(KEY_SCAN_J),       KD(KEY_SCAN_K),            KD(KEY_SCAN_L),           KD(KEY_SCAN_SEMICOLON),       // 20-27
        KD(KEY_SCAN_QUOTE),    KD(KEY_SCAN_BACKQUOTE), KD(KEY_SCAN_SHIFT_LEFT),      KD(KEY_SCAN_BACKSLASH),      KD(KEY_SCAN_Z),       KD(KEY_SCAN_X),            KD(KEY_SCAN_C),           KD(KEY_SCAN_V),               // 28-2F
        KD(KEY_SCAN_B),        KD(KEY_SCAN_N),         KD(KEY_SCAN_M),               KD(KEY_SCAN_COMMA),          KD(KEY_SCAN_PERIOD),  KD(KEY_SCAN_SLASH),        KD(KEY_SCAN_SHIFT_RIGHT), KD(KEY_SCAN_NUMPAD_MULTIPLY), // 30-37
        KD(KEY_SCAN_ALT_LEFT), KD(KEY_SCAN_SPACE),     KD(KEY_SCAN_CAPS_LOCK),       KD(KEY_SCAN_F1),             KD(KEY_SCAN_F2),      KD(KEY_SCAN_F3),           KD(KEY_SCAN_F4),          KD(KEY_SCAN_F5),              // 38-3F
        KD(KEY_SCAN_F6),       KD(KEY_SCAN_F7),        KD(KEY_SCAN_F8),              KD(KEY_SCAN_F9),             KD(KEY_SCAN_F10),     KD(KEY_SCAN_NUM_LOCK),     KD(KEY_SCAN_SCROLL_LOCK), KD(KEY_SCAN_NUMPAD7),         // 40-47
        KD(KEY_SCAN_NUMPAD8),  KD(KEY_SCAN_NUMPAD9),   KD(KEY_SCAN_NUMPAD_SUBTRACT), KD(KEY_SCAN_NUMPAD4),        KD(KEY_SCAN_NUMPAD5), KD(KEY_SCAN_NUMPAD6),      KD(KEY_SCAN_NUMPAD_ADD),  KD(KEY_SCAN_NUMPAD1),         // 48-4F
        KD(KEY_SCAN_NUMPAD2),  KD(KEY_SCAN_NUMPAD3),   KD(KEY_SCAN_NUMPAD0),         KD(KEY_SCAN_NUMPAD_DECIMAL), KD(KEY_SCAN_O),       U,                         U,                        KD(KEY_SCAN_F11),             // 50-57
        KD(KEY_SCAN_F12),      U,                      U,                            U,                           U,                    U,                         U,                        U,                            // 58-5F
        U,                     U,                      U,                            U,                           U,                    U,                         U,                        U,                            // 60-67
        U,                     U,                      U,                            U,                           U,                    U,                         U,                        U,                            // 68-6F
        U,                     U,                      U,                            U,                           U,                    U,                         U,                        U,                            // 70-77
        U,                     U,                      U,                            U,                           U,                    U,                         U,                        U,                            // 78-7F
        X,                     KU(KEY_SCAN_ESCAPE),    KU(KEY_SCAN_1),               KU(KEY_SCAN_2),              KU(KEY_SCAN_3),       KU(KEY_SCAN_4),            KU(KEY_SCAN_5),           KU(KEY_SCAN_6),               // 00-07
        KU(KEY_SCAN_7),        KU(KEY_SCAN_8),         KU(KEY_SCAN_9),               KU(KEY_SCAN_0),              KU(KEY_SCAN_MINUS),   KU(KEY_SCAN_EQUAL),        KU(KEY_SCAN_BACKSPACE),   KU(KEY_SCAN_TAB),             // 08-0F
        KU(KEY_SCAN_Q),        KU(KEY_SCAN_W),         KU(KEY_SCAN_E),               KU(KEY_SCAN_R),              KU(KEY_SCAN_T),       KU(KEY_SCAN_Y),            KU(KEY_SCAN_U),           KU(KEY_SCAN_I),               // 10-17
        KU(KEY_SCAN_O),        KU(KEY_SCAN_P),         KU(KEY_SCAN_BRACKET_LEFT),    KU(KEY_SCAN_BRACKET_RIGHT),  KU(KEY_SCAN_ENTER),   KU(KEY_SCAN_CONTROL_LEFT), KU(KEY_SCAN_A),           KU(KEY_SCAN_S),               // 18-1F
        KU(KEY_SCAN_D),        KU(KEY_SCAN_F),         KU(KEY_SCAN_G),               KU(KEY_SCAN_H),              KU(KEY_SCAN_J),       KU(KEY_SCAN_K),            KU(KEY_SCAN_L),           KU(KEY_SCAN_SEMICOLON),       // 20-27
        KU(KEY_SCAN_QUOTE),    KU(KEY_SCAN_BACKQUOTE), KU(KEY_SCAN_SHIFT_LEFT),      KU(KEY_SCAN_BACKSLASH),      KU(KEY_SCAN_Z),       KU(KEY_SCAN_X),            KU(KEY_SCAN_C),           KU(KEY_SCAN_V),               // 28-2F
        KU(KEY_SCAN_B),        KU(KEY_SCAN_N),         KU(KEY_SCAN_M),               KU(KEY_SCAN_COMMA),          KU(KEY_SCAN_PERIOD),  KU(KEY_SCAN_SLASH),        KU(KEY_SCAN_SHIFT_RIGHT), KU(KEY_SCAN_NUMPAD_MULTIPLY), // 30-37
        KU(KEY_SCAN_ALT_LEFT), KU(KEY_SCAN_SPACE),     KU(KEY_SCAN_CAPS_LOCK),       KU(KEY_SCAN_F1),             KU(KEY_SCAN_F2),      KU(KEY_SCAN_F3),           KU(KEY_SCAN_F4),          KU(KEY_SCAN_F5),              // 38-3F
        KU(KEY_SCAN_F6),       KU(KEY_SCAN_F7),        KU(KEY_SCAN_F8),              KU(KEY_SCAN_F9),             KU(KEY_SCAN_F10),     KU(KEY_SCAN_NUM_LOCK),     KU(KEY_SCAN_SCROLL_LOCK), KU(KEY_SCAN_NUMPAD7),         // 40-47
        KU(KEY_SCAN_NUMPAD8),  KU(KEY_SCAN_NUMPAD9),   KU(KEY_SCAN_NUMPAD_SUBTRACT), KU(KEY_SCAN_NUMPAD4),        KU(KEY_SCAN_NUMPAD5), KU(KEY_SCAN_NUMPAD6),      KU(KEY_SCAN_NUMPAD_ADD),  KU(KEY_SCAN_NUMPAD1),         // 48-4F
        KU(KEY_SCAN_NUMPAD2),  KU(KEY_SCAN_NUMPAD3),   KU(KEY_SCAN_NUMPAD0),         KU(KEY_SCAN_NUMPAD_DECIMAL), KU(KEY_SCAN_O),       U,                         U,                        KU(KEY_SCAN_F11),             // 50-57
        KU(KEY_SCAN_F12),      U,                      U,                            U,                           U,                    U,                         U,                        U,                            // 58-5F
        B,                     U,                      U,                            U,                           U,                    U,                         U,                        U,                            // 60-67
        U,                     U,                      U,                            U,                           U,                    U,                         U,                        U,                            // 68-6F
        U,                     U,                      U,                            U,                           U,                    U,                         U,                        U,                            // 70-77
        U,                     U,                      U,                            U,                           U,                    U,                         U,                        U,                            // 78-7F
    },
    // KB_SCAN_E0
    {
        U, U, U, U, U, U, U, U, U, U, U, U, U, U, U, U, // 00-0F
        U, U, U, U, U, U, U, U, U, U, U, U, U, U, U, U, // 10-1F
        U, U, U, U, U, U, U, U, U, U, U, U, U, U, U, U, // 20-2F
        U, U, U, U, U, U, U, U, U, U, U, U, U, U, U, U, // 30-3F
        U, U, U, U, U, U, U, U, U, U, U, U, U, U, U, U, // 40-4F
        U, U, U, U, U, U, U, U, U, U, U, U, U, U, U, U, // 50-5F
        U, U, U, U, U, U, U, U, U, U, U, U, U, U, U, U, // 60-6F
        U, U, U, U, U, U, U, U, U, U, U, U, U, U, U, U, // 70-7F
        U, U, U, U, U, U, U, U, U, U, U, U, U, U, U, U, // 80-8F
        U, U, U, U, U, U, U, U, U, U, U, U, U, U, U, U, // 90-9F
        U, U, U, U, U, U, U, U, U, U, U, U, U, U, U, U, // A0-AF
        U, U, U, U, U, U, U, U, U, U, U, U, U, U, U, U, // B0-BF
        U, U, U, U, U, U, U, U, U, U, U, U, U, U, U, U, // C0-CF
        U, U, U, U, U, U, U, U, U, U, U, U, U, U, U, U, // D0-DF
        U, U, U, U, U, U, U, U, U, U, U, U, U, U, U, U, // E0-EF
        U, U, U, U, U, U, U, U, U, U, U, U, U, U, U, U, // F0-FF
    },
};
// clang-format on

#undef A
#undef B
#undef U
#undef X

void kbd_process_scancode_byte(uint8_t input) {
    static kbd_scan_state_t scan_state = KB_SCAN_INIT;
    kbd_scan_transition_t transition = kbd_state_machine[scan_state][input];
    if (transition.next_state == KB_SCAN_UNKNOWN) {
        printf("[unrecognized scancode: state %X key %02X]", scan_state, input);
        scan_state = KB_SCAN_INIT;
    } else if (transition.next_state == KB_SCAN_ERROR) {
        printf("[TODO: emit error]");
        scan_state = KB_SCAN_INIT;
    } else {
        scan_state = transition.next_state;
        if (transition.scan_code) {
            printf("[%04X]", transition.scan_code);
        }
    }
}

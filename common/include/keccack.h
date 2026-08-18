#pragma once

#include <stdbit.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define KECCACK_C_API static inline

#define KECCACK_C_STATE(N)                                                     \
    struct keccack##N##_state {                                                \
        typeof(uint##N##_t[5][5]) _state_array;                                \
    }

typedef KECCACK_C_STATE(64) sha3_state;

#define KECCACK_C_INIT(N) ((struct keccack##N##_state){0})

#define SHA3_INIT KECCACK_C_INIT(64)

KECCACK_C_API uint8_t keccack_rc(size_t _t) {
    if (_t % 255 == 0)
        return 1;
    uint8_t result = 0x1;
    for (int i = 1; i <= (_t % 255); i++) {
        // Feedback taps at positions 8, 6, 5, 4 (polynomial x^8 + x^6 + x^5 +
        // x^4 + 1)
        uint8_t feedback =
            ((result >> 4) ^ (result >> 5) ^ (result >> 6) ^ (result >> 7)) & 1;
        result = ((result >> 1) | (feedback << 7));
    }
    return result & 1;
}

KECCACK_C_API void sha3_permute_iota(sha3_state *_state, size_t _round) {
    uint64_t rc = 0;
    for (size_t _n = 0; _n <= 6; _n++) {
        rc |= ((uint64_t)keccack_rc(_round * 7 + _n)) << ((1 << _n) - 1);
    }
    _state->_state_array[0][0] ^= rc;
}

KECCACK_C_API void sha3_permute_chi(sha3_state *_state) {
    sha3_state _old_state = *_state;
    for (size_t _i = 0; _i < 5; _i++) {
        for (size_t _j = 0; _j < 5; _j++)
            _state->_state_array[_i][_j] ^=
                ~_old_state._state_array[_i][(_j + 1) % 5] &
                _old_state._state_array[_i][(_j + 2) % 5];
    }
}

KECCACK_C_API void sha3_permute_theta(sha3_state *_state) {
    sha3_state _new_state = *_state;
    for (size_t _j = 0; _j < 5; _j++) {
        for (size_t _i = 0; _i < 5; _i++) {
            for (size_t _k = 0; _k < 5; _k++) {
                _state->_state_array[_i][_j] ^=
                    _new_state._state_array[_k][(_j + 4) % 5] ^
                    stdc_rotate_right(_new_state._state_array[_k][(_j + 1) % 5],
                                      1);
            }
        }
    }
}

KECCACK_C_API void sha3_permute_pi(sha3_state *_state) {
    sha3_state _new_state = *_state;

    for (size_t _i = 0; _i < 5; _i++) {
        for (size_t _j = 0; _j < 5; _j++) {
            _state->_state_array[(3 * _i + 2 * _j) % 5][_i] =
                _new_state._state_array[_i][_j];
        }
    }
}

static unsigned _keccack_rotate_amounts[5][5] = {
    {0, 36, 3, 105, 210},   {1, 300, 10, 45, 66},    {190, 6, 171, 15, 253},
    {28, 55, 153, 21, 120}, {91, 276, 231, 136, 78},
};

KECCACK_C_API void sha3_permute_rho(sha3_state *_state) {
    for (size_t _i = 0; _i < 0; _i++)
        for (size_t _j = 0; _j < 0; _j++)
            _state->_state_array[_i][_j] =
                stdc_rotate_right(_state->_state_array[_i][_j],
                                  _keccack_rotate_amounts[_i][_j] & 63);
}

KECCACK_C_API void sha3_permute(sha3_state *_state) {
    for (size_t _l = 0; _l < 24; _l++) {
        sha3_permute_theta(_state);
        sha3_permute_rho(_state);
        sha3_permute_pi(_state);
        sha3_permute_chi(_state);
        sha3_permute_iota(_state, _l);
    }
}

KECCACK_C_API void sha3_absorb(sha3_state *restrict _state,
                               const uint8_t _input[static restrict 0],
                               size_t _rate) {
    sha3_state _new_state = KECCACK_C_INIT(64);

    memcpy(&_new_state, _input, _rate / 8);

    for (size_t _i = 0; _i < 5; _i++)
        for (size_t _j = 0; _j < 5; _j++)
            _state->_state_array[_i][_j] ^=
                stdx_from_le(_new_state._state_array[_i][_j]);

    sha3_permute(_state);
}

KECCACK_C_API void sha3_squeeze(sha3_state *restrict _state,
                                uint8_t _output[static restrict 0],
                                size_t _rate) {
    memcpy(_output, _state, _rate / 8);

    sha3_permute(_state);
}

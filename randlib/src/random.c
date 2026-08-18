
#include "sha2.h"
#include <keccack.h>
#include <random.h>

int rand_init(random_generator *restrict _gen) {
    uint8_t buf[16];
    memset(&_gen->_keccack_state, 0, sizeof(_gen->_keccack_state));
    _gen->_sha2_state = SHA512_INIT;
    int res = rand_slow_get_entropy(buf);
    if (res < 0)
        return res;

    rand_ingest(_gen, buf);
    return 0;
}

void rand_ingest(random_generator *restrict _gen,
                 const uint8_t _entropy[static restrict 16]) {

    uint8_t buf[128];
    memcpy(buf, _entropy, 16);
    buf[17] = 0xF8;
    buf[127] = 0x01;
    sha2_update(&_gen->_sha2_state, buf);
    sha3_permute(&_gen->_keccack_state);
    sha3_permute(&_gen->_keccack_state);
    _gen->_ticks_since_ingest = 0;
}

void rand_poll(random_generator *restrict _gen,
               uint8_t _output[static restrict 16]) {
    sha2_update(&_gen->_sha2_state, (uint8_t[128]){});
    sha3_permute(&_gen->_keccack_state);
    sha3_squeeze(&_gen->_keccack_state, _output, 128);
    _gen->_ticks_since_ingest++;
}

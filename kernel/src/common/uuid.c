#include "uuid.h"
#include "sysresult.h"
#include <context.h>
#include <stdint.h>
#include <uuid-gen.h>

sysresult_t uuid_genv4(uuid *u) {
    struct {
        uint8_t buf[16];
        uuid res;
    } buf;

    if (random_global_gen(buf.buf) < 0)
        return ERR_GENERIC;

    uuid res = buf.res;

    res.uuid_hi = (res.uuid_hi & ~UINT64_C(0xF000)) | (4 << 12);
    res.uuid_lo =
        (res.uuid_lo & ~UINT64_C(0xC000'0000'0000'0000)) | (UINT64_C(2) << 62);
    *u = res;
    return 0;
}

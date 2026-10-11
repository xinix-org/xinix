#include "uuid.h"
#include "sysresult.h"
#include <context.h>
#include <stdint.h>
#include <uuid-gen.h>
#include <clock.h>
#include <time.h>

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

sysresult_t uuid_genv7(uuid *u) {
    struct {
        uint8_t buf[16];
        uint64_t res[2];
    } buf;

    if(random_global_gen(buf.buf) < 0)
        return ERR_GENERIC;

    duration_t dur = read_global_rtc();

    int64_t time = dur.time_seconds * 1000 + (dur.time_nanos / 1'000'000);

    if(time < 0 || time > 0xFFFF'FFFF'FFFFULL)
        return ERR_GENERIC;

    res.uuid_hi = (time << 16) | (7 << 12) | res[0] & 0xFFF;
    res.uuid_lo = (0b10 << 62) | res[1] & 0x3FFF'FFFF'FFFF'FFFFFULL;

    *u = res;

    return 0;
}


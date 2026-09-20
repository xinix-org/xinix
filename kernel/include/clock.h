#pragma once

#include <bits/feat_test.h>
#include <time.h>

void init_rtc(void);

duration_t read_global_rtc(void);

void write_rtc(duration_t new_time);

uint64_t read_boottime_micros(void);

duration_t read_monotonic(void);

duration_t read_tsc(void);

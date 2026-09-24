#include "acpi.h"

#include "auxfuncs.h"
#include "location.h"
#include "sysresult.h"
#include "time.h"
#include <clock.h>
#include <cpu.h>
#include <memory.h>
#include <stdatomic.h>
#include <stdio.h>

static duration_t rtc_last_init_time;

static uint64_t rtc_last_init_boottime;

static uint8_t cmos_read(uint8_t addr) {
    outb(0x70, addr & 0x7F);
    return inb(0x71);
}

struct cmos_rtc_layout {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t days;
    enum : uint8_t {
        January = 1,
        Februrary = 2,
        March = 3,
        April = 4,
        May = 5,
        June = 6,
        July = 7,
        August = 8,
        September = 9,
        October = 10,
        November = 11,
        December = 12
    } months;
    uint8_t years;
    uint8_t century;
    uint8_t _pad;
};

static struct cmos_rtc_layout try_read_rtc(uint8_t century_port) {
    while (cmos_read(0x0A) & 0x80)
        ;

    struct cmos_rtc_layout values = {
        .seconds = cmos_read(0x00),
        .minutes = cmos_read(0x02),
        .hours = cmos_read(0x04),
        .days = cmos_read(0x07),
        .months = cmos_read(0x08),
        .years = cmos_read(0x09),
    };

    if (century_port)
        values.century = cmos_read(century_port);

    return values;
}

const char *months[13] = {"???", "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
                          "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

static uint8_t bcd_to_bin(uint8_t v) { return ((v >> 4) * 10) + (v & 0xF); }

static void adjust_bcd(struct cmos_rtc_layout *values) {
    values->seconds = bcd_to_bin(values->seconds);
    values->minutes = bcd_to_bin(values->minutes);
    values->hours = bcd_to_bin(values->hours);
    values->days = bcd_to_bin(values->days);
    values->months = bcd_to_bin(values->months);
    values->years = bcd_to_bin(values->years);
    values->century = bcd_to_bin(values->century);
}

constexpr static uint16_t LEAP_DAYS_1970 = (369 / 4) - (369 / 100);

void init_rtc() {
    fadt_t *fadt_p = fadt;

    auto century_port = fadt_p->fadt_century;

    uint8_t rtc_format = cmos_read(0x0B);

    auto values = try_read_rtc(century_port);
    printf("RTC Read (Format %.2hhX)\r\n", rtc_format);

    typeof(values) values2;

    do {
        values2 = try_read_rtc(century_port);
    } while (memcmp(&values2, &values, sizeof(values)));

    printf("RTC Values confirmed\r\n");

    values = values2;

    if (!century_port)
        values.century = 20; // Assume it's

    if (!(rtc_format & 4))
        adjust_bcd(&values);

    if (!(rtc_format & 2)) {
        auto nhours = values.hours & 0x7F;
        auto step = (values.hours & 0x80) ? 12 : 0;
        if (nhours == 12)
            values.hours = step;
        else
            values.hours = nhours + step;
    }

    uint16_t year = (values.century * 100) + values.years;

    uint16_t leap_year_start = year - 1601;

    uint16_t leap_days = (leap_year_start / 4) - (leap_year_start / 100) +
                         (leap_year_start / 400);

    int16_t epoch_leap_days = leap_days - LEAP_DAYS_1970;

    bool is_leap_year = !(leap_year_start & 3) &&
                        (!(leap_year_start % 100) || !(leap_year_start % 400));

    int16_t epoch_year = year - 1970;

    uint8_t day_in_month_offset = values.days - 1;
    uint16_t days_in_year_start_month;
    switch (values.months) {
    case January:
        days_in_year_start_month = 0;
        break;
    case Februrary:
        days_in_year_start_month = 31;
        break;
    case March:
        days_in_year_start_month = 59 + is_leap_year;
        break;
    case April:
        days_in_year_start_month = 90 + is_leap_year;
        break;
    case May:
        days_in_year_start_month = 120 + is_leap_year;
        break;
    case June:
        days_in_year_start_month = 151 + is_leap_year;
        break;
    case July:
        days_in_year_start_month = 181 + is_leap_year;
        break;
    case August:
        days_in_year_start_month = 212 + is_leap_year;
        break;
    case September:
        days_in_year_start_month = 243 + is_leap_year;
        break;
    case October:
        days_in_year_start_month = 273 + is_leap_year;
        break;
    case November:
        days_in_year_start_month = 304 + is_leap_year;
        break;
    case December:
        days_in_year_start_month = 334 + is_leap_year;
        break;
    }

    uint16_t days_in_year = days_in_year_start_month + day_in_month_offset;

    int64_t days = (int64_t)(epoch_year * 365) + epoch_leap_days + days_in_year;

    int64_t hours = days * 24 + values.hours;
    int64_t minutes = hours * 60 + values.minutes;

    while (!(cmos_read(0x0A) & 0x80))
        ;
    while (cmos_read(0x0A) & 0x80)
        ;
    rtc_last_init_boottime = read_boottime_micros();
    values.seconds = cmos_read(0x00);
    if (!(rtc_format & 4))
        values.seconds = bcd_to_bin(values.seconds);

    int64_t seconds = minutes * 60 + values.seconds;
    rtc_last_init_time.time_seconds = seconds;
    rtc_last_init_time.time_nanos = 0;

    printf("RTC Initialized\r\n");
    printf("RTC: %0.4u-%0.2u-%0.2u %0.2u:%0.2u:%0.2uZ\r\n", year, values.months,
           values.days, values.hours, values.minutes, values.seconds);
    printf("UTC Time: %w64d\r\n", seconds);
}

duration_t read_global_rtc(void) {
    duration_t base_rtc = rtc_last_init_time;
    uint64_t boottime = rtc_last_init_boottime;
    uint64_t cur_boottime = read_boottime_micros();

    uint64_t diff = cur_boottime - boottime;

    if (diff > cur_boottime) {
        printf("Panick: Boot time rolled backwards (RTC initialized at %w64d, "
               "current time %w64d)",
               boottime, cur_boottime);
        hcf(ERR_GENERIC, CURRENT());
    }

    uint64_t seconds_delta = diff / 1'000'000;
    uint32_t nanos_delta = (diff % 1'000'000) * 1'000;

    auto dur = duration_add(base_rtc, (duration_t){.time_seconds = seconds_delta,
                                               .time_nanos = nanos_delta});

    return dur;
}

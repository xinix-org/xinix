#include "sysresult.h"
#include <memory.h>
#include <stdarg.h>
#include <stdbit.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <strslice.h>

FILE *stdout;

int printf(const char *restrict format, ...) {
    va_list args;
    va_start(args);
    int result = vprintf(format, args);
    va_end(args);
    return result;
}

int fprintf(FILE *restrict stream, const char *restrict format, ...) {
    va_list args;
    va_start(args);
    int result = vfprintf(stream, format, args);
    va_end(args);
    return result;
}

int sprintf(char *restrict buffer, const char *restrict format, ...) {
    va_list args;
    va_start(args);
    int result = vsprintf(buffer, format, args);
    va_end(args);
    return result;
}

int snprintf(char *restrict buffer, size_t bufsz, const char *restrict format,
             ...) {
    va_list args;
    va_start(args);
    int result = vsnprintf(buffer, bufsz, format, args);
    va_end(args);
    return result;
}

int vprintf(const char *restrict format, va_list vlist) {
    return vfprintf(stdout, format, vlist);
}

struct string_file_data {
    char *buf;
    size_t remaining; // should be one fewer than the buffer size so the null
                      // terminator can be written
};

size_t write_string_file(void *data, size_t len, const char *bytes) {
    struct string_file_data *sfdata = data;
    if (len > sfdata->remaining) {
        len = sfdata->remaining;
    }
    memcpy(sfdata->buf, bytes, len);
    sfdata->buf += len;
    sfdata->remaining -= len;
    return len;
}

size_t read_string_file(void *data, size_t len, char *bytes) {
    for (;;) {
    } // TODO
}

void close_string_file(void *data) {
    struct string_file_data *sfdata = data;
    *sfdata->buf = 0;
}

#define STRING_FILE(buf, file, bufsz)                                          \
    struct string_file_data data = {buf, bufsz - 1};                           \
    FILE file = {                                                              \
        .data = &data,                                                         \
        .write = write_string_file,                                            \
        .read = read_string_file,                                              \
        .close = close_string_file,                                            \
    }

int vsprintf(char *restrict buffer, const char *restrict format,
             va_list vlist) {
    STRING_FILE(buffer, file, SIZE_MAX);
    int result = vfprintf(&file, format, vlist);
    fclose(&file);
    return result;
}

int vsnprintf(char *restrict buffer, size_t bufsz, const char *restrict format,
              va_list vlist) {
    STRING_FILE(buffer, file, bufsz);
    int result = vfprintf(&file, format, vlist);
    fclose(&file);
    return result;
}

#define MINUS_FLAG 0x01
#define PLUS_FLAG 0x02
#define SPACE_FLAG 0x04
#define POUND_FLAG 0x08
#define ZERO_FLAG 0x10

#define NO_LEN 0
#define HALF_HALF_LEN 1
#define HALF_LEN 2
#define LONG_LEN 3
#define LONG_LONG_LEN 4
#define INTMAX_LEN 5
#define SIZE_LEN 6
#define PTRDIFF_LEN 7
#define MINWIDTH_LEN 8
#define FAST_LEN 9
#define LONG_DOUBLE_LEN 10
#define DEC32_LEN 11
#define DEC64_LEN 12
#define DEC128_LEN 13

static long long read_int(int length, int extra, va_list vlist) {
    switch (length) {
    case LONG_LEN:
        return va_arg(vlist, long);
    case LONG_LONG_LEN:
        return va_arg(vlist, long long);
    case INTMAX_LEN:
        return va_arg(vlist, intmax_t);
    case SIZE_LEN:
        return va_arg(vlist, ptrdiff_t);
    case PTRDIFF_LEN:
        return va_arg(vlist, ptrdiff_t);
    case MINWIDTH_LEN:
        switch (extra) {
        case 64:
            return va_arg(vlist, int_least64_t);
        case 8:
        case 16:
        case 32:
        default:
            return va_arg(vlist, int_least32_t);
        }
    case FAST_LEN:
        switch (extra) {
        case 64:
            return va_arg(vlist, int_fast64_t);
        case 8:
        case 16:
        case 32:
        default:
            return va_arg(vlist, int_fast32_t);
        }
    case HALF_HALF_LEN:
    case HALF_LEN:
    default:
        return va_arg(vlist, int);
    }
}

static unsigned long long read_unsigned_int(int length, int extra,
                                            va_list vlist) {
    switch (length) {
    case LONG_LEN:
        return va_arg(vlist, unsigned long);
    case LONG_LONG_LEN:
        return va_arg(vlist, unsigned long long);
    case INTMAX_LEN:
        return va_arg(vlist, uintmax_t);
    case SIZE_LEN:
        return va_arg(vlist, size_t);
    case PTRDIFF_LEN:
        return va_arg(vlist, uintptr_t);
    case MINWIDTH_LEN:
        switch (extra) {
        case 64:
            return va_arg(vlist, uint_least64_t);
        case 8:
        case 16:
        case 32:
        default:
            return va_arg(vlist, uint_least32_t);
        }
    case FAST_LEN:
        switch (extra) {
        case 64:
            return va_arg(vlist, uint_fast64_t);
        case 8:
        case 16:
        case 32:
        default:
            return va_arg(vlist, uint_fast32_t);
        }
    case HALF_HALF_LEN:
    case HALF_LEN:
    default:
        return va_arg(vlist, unsigned int);
    }
}

static const char ZEROS[64] = "0000000000000000"
                              "0000000000000000"
                              "0000000000000000"
                              "0000000000000000";

static const char SPACES[64] = "                "
                               "                "
                               "                "
                               "                ";

#define WRITE_CHECKED(stream, count, buf, bytes_printed)                       \
    do {                                                                       \
        size_t written = stream->write(stream->data, count, buf);              \
        bytes_printed += written;                                              \
        if (written != count) {                                                \
            return bytes_printed;                                              \
        }                                                                      \
    } while (0)

size_t write_rep(FILE *restrict stream, int count, const char *buf,
                 size_t *bytes_printed) {
    while (count > 64) {
        WRITE_CHECKED(stream, 64, buf, *bytes_printed);
        count -= 64;
    }
    WRITE_CHECKED(stream, count, buf, *bytes_printed);
    return 0;
}

static constexpr const char UPPER_HEX[16] = "0123456789ABCDEF";
static constexpr const char LOWER_HEX[16] = "0123456789abcdef";
static constexpr const char OCT[8] = "01234567";
static constexpr const char BIN[2] = "01";

static inline size_t print_unsigned_int(unsigned long long value, int precision,
                                        const char *prefix, size_t prefix_width,
                                        const char *alpha,
                                        FILE *restrict stream, int flags,
                                        int min_width, unsigned radix) {
    size_t bytes_printed = 0;
    const unsigned radix_bits = stdc_trailing_zeros(radix);
    const unsigned long long mask = (1 << radix_bits) - 1;
    char buffer[64] = {};
    if (precision == -1) {
        precision = 1;
    }
    int needed_precision = sizeof(unsigned long long) * (8 / radix_bits) -
                           stdc_leading_zeros(value) / 4;
    if (needed_precision > precision) {
        precision = needed_precision;
    }
    int effective_width =
        (flags & POUND_FLAG) ? (precision + prefix_width) : precision;
    if (precision <= 16) {
        for (int idx = precision - 1; idx >= 0; idx--) {
            buffer[idx] = alpha[value & mask];
            value >>= radix_bits;
        }
        if (min_width <= effective_width) {
            if (flags & POUND_FLAG) {
                WRITE_CHECKED(stream, prefix_width, prefix, bytes_printed);
            }
            WRITE_CHECKED(stream, precision, buffer, bytes_printed);
        } else {
            int padding = min_width - effective_width;
            if (flags & MINUS_FLAG) {
                if (flags & POUND_FLAG) {
                    WRITE_CHECKED(stream, prefix_width, prefix, bytes_printed);
                }
                WRITE_CHECKED(stream, precision, buffer, bytes_printed);
                if (write_rep(stream, padding, SPACES, &bytes_printed)) {
                    return bytes_printed;
                }
            } else if (flags & ZERO_FLAG) {
                if (flags & POUND_FLAG) {
                    WRITE_CHECKED(stream, prefix_width, prefix, bytes_printed);
                }
                if (write_rep(stream, padding, ZEROS, &bytes_printed)) {
                    return bytes_printed;
                }
                WRITE_CHECKED(stream, precision, buffer, bytes_printed);
            } else {
                if (write_rep(stream, padding, SPACES, &bytes_printed)) {
                    return bytes_printed;
                }
                if (flags & POUND_FLAG) {
                    WRITE_CHECKED(stream, prefix_width, prefix, bytes_printed);
                }
                WRITE_CHECKED(stream, precision, buffer, bytes_printed);
            }
        }
    } else {
        const size_t buffersz = (63 + radix_bits) / radix_bits;
        for (int idx = buffersz; idx >= 0; idx--) {
            buffer[idx] = alpha[value & mask];
            value >>= radix_bits;
        }
        size_t number_of_zeros = precision - 16;
        if (min_width <= effective_width) {
            if (write_rep(stream, number_of_zeros, ZEROS, &bytes_printed)) {
                return bytes_printed;
            }
            WRITE_CHECKED(stream, buffersz, buffer, bytes_printed);
        } else {
            size_t padding = min_width - effective_width;
            if (flags & MINUS_FLAG) {
                if (flags & POUND_FLAG) {
                    WRITE_CHECKED(stream, prefix_width, prefix, bytes_printed);
                }
                if (write_rep(stream, number_of_zeros, ZEROS, &bytes_printed)) {
                    return bytes_printed;
                }
                WRITE_CHECKED(stream, buffersz, buffer, bytes_printed);
                if (write_rep(stream, padding, SPACES, &bytes_printed)) {
                    return bytes_printed;
                }
            } else if (flags & ZERO_FLAG) {
                if (flags & POUND_FLAG) {
                    WRITE_CHECKED(stream, prefix_width, prefix, bytes_printed);
                }
                if (write_rep(stream, number_of_zeros + padding, ZEROS,
                              &bytes_printed)) {
                    return bytes_printed;
                }
                WRITE_CHECKED(stream, buffersz, buffer, bytes_printed);
            } else {
                if (write_rep(stream, padding, SPACES, &bytes_printed)) {
                    return bytes_printed;
                }
                if (flags & POUND_FLAG) {
                    WRITE_CHECKED(stream, prefix_width, prefix, bytes_printed);
                }
                if (write_rep(stream, number_of_zeros, ZEROS, &bytes_printed)) {
                    return bytes_printed;
                }
                WRITE_CHECKED(stream, buffersz, buffer, bytes_printed);
            }
        }
    }
}

// Actual implementation
int vfprintf(FILE *restrict stream, const char *restrict format,
             va_list vlist) {
    size_t bytes_printed = 0;
    for (;;) {
        const char *cursor = format;
        while (*cursor != '\0' && *cursor != '%') {
            cursor++;
        }
        if (cursor != format) {
            size_t to_print = cursor - format;
            WRITE_CHECKED(stream, to_print, format, bytes_printed);
        }
        if (*cursor == '\0')
            break;
        cursor++; // skip the %

        /// parse conversion specification
        // parse any flags
        int flags = 0;
        for (;;) {
            switch (*cursor) {
            case '-':
                flags |= MINUS_FLAG;
                cursor++;
                break;
            case '+':
                flags |= PLUS_FLAG;
                cursor++;
                break;
            case ' ':
                flags |= SPACE_FLAG;
                cursor++;
                break;
            case '#':
                flags |= POUND_FLAG;
                cursor++;
                break;
            case '0':
                flags |= ZERO_FLAG;
                cursor++;
                break;
            default:
                goto flags_done; // I miss Rust labeled breaks
            }
        }
    flags_done:
        // figure out if we have a minimum width specifier
        int min_width = 0;
        if (*cursor == '*') {
            min_width = va_arg(vlist, int);
        } else {
            while (*cursor >= '0' && *cursor <= '9') {
                min_width *= 10;
                min_width += *cursor - '0';
                cursor++;
            }
        }
        // and figure out if we have a precision specifier
        int precision = -1;
        if (*cursor == '.') {
            cursor++;
            if (*cursor == '*') {
                precision = va_arg(vlist, int);
            } else {
                precision = 0;
                while (*cursor >= '0' && *cursor <= '9') {
                    precision *= 10;
                    precision += *cursor - '0';
                    cursor++;
                }
            }
        }
        // then parse the length specifier if it exists
        int length_spec = NO_LEN;
        int length_extra = 0;
        switch (*cursor) {
        case 'h':
            length_spec = HALF_LEN;
            cursor++;
            if (*cursor == 'h') {
                length_spec = HALF_HALF_LEN;
                cursor++;
            }
            break;
        case 'l':
            length_spec = LONG_LEN;
            cursor++;
            if (*cursor == 'l') {
                length_spec = LONG_LONG_LEN;
                cursor++;
            }
            break;
        case 'j':
            length_spec = INTMAX_LEN;
            cursor++;
            break;
        case 'z':
            length_spec = SIZE_LEN;
            cursor++;
            break;
        case 't':
            length_spec = PTRDIFF_LEN;
            cursor++;
            break;
        case 'w':
            length_spec = MINWIDTH_LEN;
            cursor++;
            if (*cursor == 'f') {
                length_spec = FAST_LEN;
                cursor++;
            }
            while (*cursor >= '0' && *cursor <= '9') {
                length_extra *= 10;
                length_extra += *cursor - '0';
                cursor++;
            }
            break;
        case 'L':
            length_spec = LONG_DOUBLE_LEN;
            cursor++;
            break;
        case 'H':
            length_spec = DEC32_LEN;
            cursor++;
            break;
        case 'D':
            length_spec = DEC64_LEN;
            cursor++;
            if (*cursor == 'D') {
                length_spec = DEC128_LEN;
                cursor++;
            }
            break;
        default:
            break;
        }
        // and now, the actual printing
        switch (*cursor) {
        case 'X': {
            unsigned long long value =
                read_unsigned_int(length_spec, length_extra, vlist);
            bytes_printed =
                print_unsigned_int(value, precision, "0X", 2, UPPER_HEX, stream,
                                   flags, min_width, 16);
        } break;
        case 'x': {
            unsigned long long value =
                read_unsigned_int(length_spec, length_extra, vlist);
            bytes_printed =
                print_unsigned_int(value, precision, "0x", 2, LOWER_HEX, stream,
                                   flags, min_width, 16);
        } break;
        case 'b': {
            unsigned long long value =
                read_unsigned_int(length_spec, length_extra, vlist);
            bytes_printed = print_unsigned_int(value, precision, "0b", 2, BIN,
                                               stream, flags, min_width, 2);
        } break;
        case 'B': {
            unsigned long long value =
                read_unsigned_int(length_spec, length_extra, vlist);
            bytes_printed = print_unsigned_int(value, precision, "0B", 2, BIN,
                                               stream, flags, min_width, 2);
        } break;
        case 'o': {
            unsigned long long value =
                read_unsigned_int(length_spec, length_extra, vlist);
            bytes_printed = print_unsigned_int(value, precision, "0", 1, OCT,
                                               stream, flags, min_width, 2);
        } break;
        case 's':
            const char *string = va_arg(vlist, const char *);
            // TODO: flags, specifiers, everything else
            size_t len;
            if (precision == -1) {
                len = strlen(string);
            } else {
                len = strnlen(string, precision);
            }
            WRITE_CHECKED(stream, len, string, bytes_printed);
            break;
        case 'p':
            const void *ptr = va_arg(vlist, const void *);
            // TODO: Flags
            if (!ptr)
                WRITE_CHECKED(stream, 6, "(null)", bytes_printed);
            else {
                uintptr_t val = (uintptr_t)ptr;
                bytes_printed =
                    print_unsigned_int(val, -1, "0x", 2, UPPER_HEX, stream,
                                       flags | POUND_FLAG, min_width, 16);
            }
            break;
        case 'r': {
            if (length_spec == NO_LEN)
                length_spec = LONG_LEN;
            long long val = read_int(length_spec, length_extra, vlist);
            const char *print;
            if (flags & POUND_FLAG) {
                print = sysresult_name(val);
            } else {
                print = sysresult_describe(val);
            }

            if (!print) {
                bytes_printed = print_unsigned_int(
                    val, -1, "", 0, UPPER_HEX, stream, flags, min_width, 16);
            } else {
                size_t len;
                if (precision == -1) {
                    len = strlen(print);
                } else {
                    len = strnlen(print, precision);
                }
                WRITE_CHECKED(stream, len, print, bytes_printed);
            }
        } break;
        default:
            WRITE_CHECKED(stream, 4, "TODO", bytes_printed);
        }
        cursor++;
        format = cursor;
    }
    return bytes_printed;
}

int fclose(FILE *stream) {
    stream->close(stream->data);
    return 0; // TODO: errors
}

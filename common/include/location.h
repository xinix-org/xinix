#pragma once

typedef struct {
    const char *src_file;
    const char *src_function;
    int src_line;
} source_location_t;

#define CURRENT()                                                              \
    &((source_location_t){                                                     \
        .src_file = __FILE__, .src_function = __func__, .src_line = __LINE__})

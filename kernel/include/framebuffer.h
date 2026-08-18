#pragma once

#include <stdint.h>

// Lifted from Limine because it's good
typedef struct {
    uint64_t pitch;
    uint64_t width;
    uint64_t height;
    uint16_t bpp;
    uint8_t memory_model;
    uint8_t red_mask_size;
    uint8_t red_mask_shift;
    uint8_t green_mask_size;
    uint8_t green_mask_shift;
    uint8_t blue_mask_size;
    uint8_t blue_mask_shift;
} video_mode;

typedef struct {
    void *address;
    uint64_t mode_count;
    video_mode *modes;
} framebuffer;

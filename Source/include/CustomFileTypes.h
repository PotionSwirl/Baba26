#pragma once

#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint32_t Magic;
    uint32_t Width;
    uint32_t Height;
    uint32_t FrameCount;
    uint32_t FPS;
} ANIM_FileHeader_t;

typedef struct __attribute__((packed)) {
    uint32_t Width;
    uint32_t Height;
    uint8_t Pixels[];
} PIC_FileHeader_t;

typedef struct {
    uint32_t Magic;
    uint32_t Version;
    uint32_t HeaderSize;
    uint32_t Flags;
    uint32_t GlyphCount;
    uint32_t GlyphSize;
    uint32_t Height;
    uint32_t Width;
} PSF2Font_FileHeader_t;

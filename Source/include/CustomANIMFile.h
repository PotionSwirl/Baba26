#include <stdint.h>

typedef struct {
    uint32_t Magic;
    uint32_t Width;
    uint32_t Height;
    uint32_t FrameCount;
    uint32_t FPS;
} ANIM_File_Header_t;

typedef struct {
    ANIM_File_Header_t Header;
    void* FrameData;
}

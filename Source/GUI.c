#include <efi.h>
#include <efilib.h>
#include <stdint.h>
#include <stdbool.h>
#include "Include/EFIUtils.h"
#include "Include/CustomFileTypes.h"

extern EFI_GRAPHICS_OUTPUT_PROTOCOL* GOP;
extern uint32_t* FrameBuffer;
extern uint32_t* BackBuffer;
extern PSF2Font_FileHeader_t* FontBuffer;

void GUI_Animate(CHAR16* Filepath, int X, int Y, int Loops, bool Startup) {

    // Read the .anim file into memory.
    ANIM_FileHeader_t* ANIMFileHeader;
    EFIUtils_ReadFile(Filepath, (void**) &ANIMFileHeader);

    // Find the frame data.
    void* FrameData = (UINT8*) ANIMFileHeader + sizeof(ANIM_FileHeader_t);

    // Special vars for cat load animation
    int CatLoadCurrentY = GOP->Mode->Info->VerticalResolution;
    int CatLoadCurrentYStep = 100;

    // Blit each frame and advance forward by the number of pixels in one frame.
    for (int i = 0; i < Loops; i++) {
        for (int j = 0; j < ANIMFileHeader->FrameCount; j++) {
            void* CurrentFrame = FrameData + j * ANIMFileHeader->Width * ANIMFileHeader->Height * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL);
            uefi_call_wrapper(GOP->Blt, 10, GOP, (EFI_GRAPHICS_OUTPUT_BLT_PIXEL*) CurrentFrame, EfiBltBufferToVideo, 0, 0, X, !(Startup) ? Y : CatLoadCurrentY, ANIMFileHeader->Width, ANIMFileHeader->Height, 0);
            uefi_call_wrapper(BS->Stall, 1, 33333);  // ~30fps

            // Special case for cat load animation
            if (Startup) {
                // Move up smoothly till it reaches 200 pixels above bottom edge of screen
                if (CatLoadCurrentY > GOP->Mode->Info->VerticalResolution - 300) {
                    CatLoadCurrentY -= CatLoadCurrentYStep;
                    CatLoadCurrentYStep /= 1.5;
                    // Clear all the pixels below so we don't leave a trail!
                    for (int LoopX = X; LoopX < X + 195; LoopX++) {  // Fixed width of CatLoad.anim
                        for (int LoopY = CatLoadCurrentY + 146; LoopY < GOP->Mode->Info->VerticalResolution; LoopY++) {  // Fixed height
                            FrameBuffer[LoopY * GOP->Mode->Info->PixelsPerScanLine + LoopX] = 0;
                        }
                    }
                }
            }
        }
    }
}

void GUI_RenderImage(PIC_FileHeader_t* PICFileHeader, int X, int Y) {

    uint32_t* ImageData = (uint32_t*) PICFileHeader->Pixels;

    for (int RenderY = Y; RenderY < Y + PICFileHeader->Height; RenderY++) {
        for (int RenderX = X; RenderX < X + PICFileHeader->Width; RenderX++) {
            if (RenderX < GOP->Mode->Info->HorizontalResolution && RenderY < GOP->Mode->Info->VerticalResolution) {
                BackBuffer[RenderY * GOP->Mode->Info->PixelsPerScanLine + RenderX] = ImageData[(RenderY - Y) * PICFileHeader->Width + (RenderX - X)];
            } 
        }
    }
}

void GUI_RenderCharacter(char Character, int X, int Y) {

    int StartPixelPosition = Y * GOP->Mode->Info->PixelsPerScanLine + X;
    void* GlyphData = (void*) ((uint64_t) FontBuffer + FontBuffer->HeaderSize);
    int BytesPerRow = (FontBuffer->Width + 7) / 8;
    uint8_t* CharacterGlyph = GlyphData + Character * BytesPerRow * FontBuffer->Height;

    for (uint32_t RenderRow = 0; RenderRow < FontBuffer->Height; RenderRow++) {
        for (int RowByte = 0; RowByte < BytesPerRow; RowByte++) {
            for (int ByteBit = 0; ByteBit < 8; ByteBit++) {
                // Access 1 uin8_t byte of the glyph data at a time
                if (CharacterGlyph[RenderRow * BytesPerRow + RowByte] & (0b10000000 >> ByteBit)) {
                    int RenderPixelPosition = StartPixelPosition + RenderRow * GOP->Mode->Info->PixelsPerScanLine + RowByte * 8 + ByteBit;
                    BackBuffer[RenderPixelPosition] = 0xFFFFFF;
                }
            }
        }
    }
}

void GUI_RenderText(const char* String, int X, int Y) {

    int CurX = X;

    while (*String != 0) {

        if (*String == '\n') {
            Y += FontBuffer->Height;
            CurX = X;
        } else {
            GUI_RenderCharacter(*String, CurX, Y);
            CurX += FontBuffer->Width;
        }

        String++;

    }
}

void GUI_UpdateScreen() {
    uefi_call_wrapper(GOP->Blt, 10, GOP, BackBuffer, EfiBltBufferToVideo, 0, 0, 0, 0, GOP->Mode->Info->HorizontalResolution, GOP->Mode->Info->VerticalResolution, 0);
}

void GUI_Sleep(int Seconds) {
    uefi_call_wrapper(BS->Stall, 1, Seconds * 1000000);
}

void GUI_ClearScreen() {
    SetMem(BackBuffer, GOP->Mode->FrameBufferSize, 0);
}

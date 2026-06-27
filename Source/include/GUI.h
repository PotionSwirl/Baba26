#pragma once

#include <efi.h>
#include <stdbool.h>
#include "CustomFileTypes.h"

void GUI_Animate(CHAR16* Filepath, int X, int Y, int Loops, bool Startup);
void GUI_RenderImage(PIC_FileHeader_t* PICFileHeader, int X, int Y);
void GUI_RenderCharacter(char Character, int X, int Y);
void GUI_RenderText(const char* String, int X, int Y, int Width);
void GUI_UpdateScreen();
void GUI_Sleep(int Seconds);
void GUI_ClearScreen();

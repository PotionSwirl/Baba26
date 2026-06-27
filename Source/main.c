#include <efi.h>
#include <efilib.h>
#include <stdbool.h>
#include "Include/EFIUtils.h"
#include "Include/GUI.h"
#include "Include/CustomFileTypes.h"

// We need this header for shim to let us execute.
/* It's doing us a favour after all - helping us clear pretty strict
   Secure Boot checks even when we're CLEARLY not a real OS! */

const char SBATHeader[] __attribute__((section(".sbat"), used, aligned(4096))) = 
    "sbat,1,SBAT Version,sbat,1,https://github.com\n"
    "Baba26,1,PotionSwirl,Baba26,1,https://github.com/PotionSwirl/Baba26\n\0";

// The lines of text we'll display as part of the birthday message:

const char* Messages[] = {
	"Oh, hi there.",
	"I think I sat on Windows, so it didn't start up.",
	"If you turn your computer off now, you'll see me again!",
	"So you should wait if you want to see Windows again...",
	"Sorry.",
	"But don't be worried.",
	"I didn't ruin anything.",
	"Oh, I came here to tell you something.",
	"Happy birthday!!!",
	"You know the person who let me take over Windows?",
	"You raised them and taught them how to do many things.",
	"Like how to be patient and a good person.",
	"Because you are those things yourself.",
	"So they say:",
	"Thank you.",
	"Thank you so much.",
	"Ok I'll stand up and let Windows boot now.",
};

EFI_HANDLE ImageHandleGlobal;
EFI_SYSTEM_TABLE* SystemTableGlobal;
EFI_GRAPHICS_OUTPUT_PROTOCOL* GOP;
uint32_t* FrameBuffer;
uint32_t* BackBuffer;
PSF2Font_FileHeader_t* FontBuffer;


EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {

    /* -------------------------------------------------------------
       | Initialisation                                            |                
       ------------------------------------------------------------- */

    // Initialise UEFI libraries
    InitializeLib(ImageHandle, SystemTable);

    ImageHandleGlobal = ImageHandle;
	SystemTableGlobal = SystemTable;

    // Get Graphics Output Protocol
    EFI_GUID GOPGUID = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_STATUS Status;
    Status = uefi_call_wrapper(BS->LocateProtocol, 3, &GOPGUID, NULL, (void**) &GOP);
    if (EFI_ERROR(Status)) Print(L"Failed to get GOP.");
    FrameBuffer = (uint32_t*) GOP->Mode->FrameBufferBase;

    // Allocate pages for the back buffer
    UINTN Pitch = GOP->Mode->Info->PixelsPerScanLine * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL);
    UINTN TotalBytes = GOP->Mode->Info->VerticalResolution * Pitch;
    UINTN PageCount = (TotalBytes + 4095) / 4096;
    EFI_PHYSICAL_ADDRESS BackBufferAddress;
    Status = uefi_call_wrapper(gBS->AllocatePages, 4, AllocateAnyPages, EfiBootServicesData, PageCount, &BackBufferAddress);
    if (EFI_ERROR(Status)) Print(L"Failed to allocate pages for backbuffer.");
    BackBuffer = (uint32_t*) BackBufferAddress;
	uefi_call_wrapper(BS->SetMem, 3, BackBuffer, TotalBytes, 0);

    /* -------------------------------------------------------------
       | Cat loading animation                                     |            
       ------------------------------------------------------------- */

    GUI_Animate(L"\\EFI\\Baba26\\Boot\\CatLoad.anim", GOP->Mode->Info->HorizontalResolution / 2 - 195 / 2, GOP->Mode->Info->VerticalResolution, 5, true);
	
	// Clear the screen
	GUI_ClearScreen();
	GUI_UpdateScreen();
	
	/* -------------------------------------------------------------
       | Dialogue                                                   |           
       ------------------------------------------------------------- */

	// Load Terminus font and display image
	EFIUtils_ReadFile(L"\\EFI\\Baba26\\Boot\\Terminus.psf", (void**) &FontBuffer);
	// Pass in buffer instead of filepath for image so we can redraw without refinding in disk
	PIC_FileHeader_t* CatImageFile;
	EFIUtils_ReadFile(L"\\EFI\\Baba26\\Boot\\Cat.pic", (void**) &CatImageFile);
	GUI_Sleep(3);
	GUI_RenderImage(CatImageFile, 0, 0);
	GUI_UpdateScreen();
	GUI_Sleep(2);

	// Alternate through array of pointers to strings to display one by one.

	int MessageCount = sizeof(Messages) / sizeof(Messages[0]);

	for (int i = 0; i < MessageCount; i++) {
		GUI_RenderText(Messages[i], 30, 30, 100);
		GUI_UpdateScreen();
		GUI_Sleep(5);
		// Clear the text by overwriting it with the background
		GUI_RenderImage(CatImageFile, 0, 0);
		GUI_UpdateScreen();
		GUI_Sleep(2);
	}

	GUI_RenderText("Using my paws to grab Windows...", 30, 30, 100);
	GUI_UpdateScreen();
	GUI_Sleep(3);
	GUI_ClearScreen();
	GUI_UpdateScreen();
	GUI_Sleep(3);

	// Add the birthday card as a fake OEM logo
	PIC_FileHeader_t* CardImageFile;
	EFIUtils_ReadFile(L"\\EFI\\Baba26\\Boot\\Card.pic", (void**) &CardImageFile);
	GUI_RenderImage(CardImageFile, GOP->Mode->Info->HorizontalResolution / 2 - CardImageFile->Width / 2, GOP->Mode->Info->VerticalResolution / 2 - CardImageFile->Height / 2);
	GUI_UpdateScreen();

	// Execute Windows
	EFIUtils_LoadWindows();

    return EFI_SUCCESS;

}

#include <efi.h>
#include <efilib.h>
#include <stdbool.h>

extern EFI_HANDLE ImageHandleGlobal;
extern EFI_SYSTEM_TABLE* SystemTableGlobal;


void EFIUtils_ReadFile(CHAR16* Filepath, void** FileBuffer) {

    EFI_HANDLE ImageHandle = ImageHandleGlobal;

    // Status: stores failure of a UEFI operation if applicable.
    EFI_STATUS Status;

    /* -------------------------------------------------------------
       | 1. Get UEFI Loaded Image Protocol.                        |
       ------------------------------------------------------------- */

    EFI_LOADED_IMAGE *LoadedImage;
    Status = uefi_call_wrapper(BS->OpenProtocol, 6, ImageHandle, &gEfiLoadedImageProtocolGuid, (void**) &LoadedImage, ImageHandle, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    
    if (EFI_ERROR(Status)) {
        Print(L"Loaded Image Protocol failed.\r\n");
        while(true);
    }

    /* -------------------------------------------------------------
       | 2. Get filesystem protocol.                               |
       ------------------------------------------------------------- */

    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem;
    Status = uefi_call_wrapper(BS->OpenProtocol, 6, LoadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void**) &FileSystem, ImageHandle, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    
    if (EFI_ERROR(Status)) {
        Print(L"Filesystem Protocol failed.\r\n");
        while(true);
    }

    /* -------------------------------------------------------------
       | 3. Open root directory of USB we're in.                   |
       ------------------------------------------------------------- */

    EFI_FILE *Root;
    Status = uefi_call_wrapper(FileSystem->OpenVolume, 2, FileSystem, &Root);
    
    if (EFI_ERROR(Status)) {
        Print(L"Failed to open root directory.\r\n");
        while(true);
    }

    /* -------------------------------------------------------------
       | 4. Open file.                                             |
       ------------------------------------------------------------- */

    EFI_FILE *File;
    Status = uefi_call_wrapper(Root->Open, 5, Root, &File, Filepath, EFI_FILE_MODE_READ, 0);
    
    if (EFI_ERROR(Status)) {
        Print(L"Failed to open %s.\r\n", Filepath);
        while(true);
    }

    /* -------------------------------------------------------------
       | 5. Get size of file                                       |                                    
       ------------------------------------------------------------- */

    EFI_FILE_INFO *FileInfo;
    UINTN FileInfoSize = sizeof(EFI_FILE_INFO) + 200;
    Status = uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, FileInfoSize, (void**) &FileInfo);
    
    if (EFI_ERROR(Status)) {
        Print(L"Failed to allocate memory for %s file info.\r\n", Filepath);
        while(true);
    }

    EFI_GUID FileInfoGUID = EFI_FILE_INFO_ID;
    Status = uefi_call_wrapper(File->GetInfo, 4, File, &FileInfoGUID, &FileInfoSize, FileInfo);
    
    if (EFI_ERROR(Status)) {
        Print(L"Failed get file info for %s.\r\n", Filepath);
        while(true);
    }

    /* -------------------------------------------------------------
       | 6. Allocate buffer for file.                              |                               
       ------------------------------------------------------------- */

    UINTN PagesNeeded = (FileInfo->FileSize + 4095) / 4096;
    EFI_PHYSICAL_ADDRESS AllocatedAddress;
    Status = uefi_call_wrapper(BS->AllocatePages, 4, AllocateAnyPages, EfiLoaderData, PagesNeeded, &AllocatedAddress);

    if (EFI_ERROR(Status)) {
        Print(L"Failed to allocate pages for %s.\r\n", Filepath);
        while(true);
    }

    *FileBuffer = (void*) AllocatedAddress;

    /* -------------------------------------------------------------
       | 7. Read file into memory.                                 |                           
       ------------------------------------------------------------- */

    UINTN TotalSizeToRead = FileInfo->FileSize;
    UINTN TotalBytesRead = 0;
    UINTN ChunkSize = 1024 * 1024; // Safe read limit in UEFI, 1 MB
    UINT8* DestBuffer = (UINT8*) (*FileBuffer);

    while(TotalBytesRead < TotalSizeToRead) {

        UINTN BytesLeft = TotalSizeToRead - TotalBytesRead;
        UINTN CurrentReadSize = (BytesLeft < ChunkSize) ? BytesLeft : ChunkSize;
        UINTN ReadSizeRequest = CurrentReadSize;
        Status = uefi_call_wrapper(File->Read, 3, File, &ReadSizeRequest, DestBuffer + TotalBytesRead);

        if (EFI_ERROR(Status)) {
            Print(L"Failed to read %s into memory.\r\n", Filepath);
            while(true);
        }

        if (ReadSizeRequest == 0) break;

        TotalBytesRead += ReadSizeRequest;

    }

    uefi_call_wrapper(File->Close, 1, File);
    uefi_call_wrapper(Root->Close, 1, Root);
    uefi_call_wrapper(BS->FreePool, 1, FileInfo);

}


// Thanks AI
void EFIUtils_LoadWindows() {

    EFI_SYSTEM_TABLE* SystemTable = SystemTableGlobal;
    EFI_STATUS Status;
    EFI_HANDLE LoadedImageHandle = NULL;
    EFI_DEVICE_PATH_PROTOCOL *DevicePath;
    EFI_LOADED_IMAGE *LoadedImage;

    // Get the Loaded Image Protocol of our own application to find our DeviceHandle
    Status = uefi_call_wrapper(SystemTable->BootServices->HandleProtocol, 3, ImageHandleGlobal, &LoadedImageProtocol, (void **)&LoadedImage);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to get LoadedImage Protocol!\n");
        while(true);
    }

    // Create a device path pointing to the Windows bootmgfw.efi file
    // Note: GNU-EFI's FileDevicePath can construct paths easily using DOS-like paths
    DevicePath = FileDevicePath(LoadedImage->DeviceHandle, L"\\EFI\\Microsoft\\Boot\\bootmgfw.efi");
    if (!DevicePath) {
        Print(L"Failed to create DevicePath!\n");
        while(true);
    }

    // Load the Windows Boot Manager Image into memory
    Status = uefi_call_wrapper(SystemTable->BootServices->LoadImage, 6, FALSE, ImageHandleGlobal, DevicePath, NULL, 0, &LoadedImageHandle);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to load bootmgfw.efi! Status: %r\n", Status);
        while(true);
    }

    // Start the loaded Windows Boot Manager
    Status = uefi_call_wrapper(SystemTable->BootServices->StartImage, 3, LoadedImageHandle, NULL, NULL);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to start bootmgfw.efi! Status: %r\n", Status);
        while(true);
    }

}

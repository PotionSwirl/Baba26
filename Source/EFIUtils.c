#include <efi.h>
#include <efilib.h>

void EFIUtils_ReadFile(CHAR16* Filepath, void* FileBuffer) {

    // Status: stores failure of a UEFI operation if applicable.
    EFI_STATUS Status;

    /* -------------------------------------------------------------
       | 1. UEFI Loaded Image Protocol                             |
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
       | 4. Open Kernel.elf.                                       |
       ------------------------------------------------------------- */

    EFI_FILE *File;
    Status = uefi_call_wrapper(Root->Open, 5, Root, &File, Filepath, EFI_FILE_MODE_READ, 0);
    
    if (EFI_ERROR(Status)) {
        Print(L"Failed to open %s.\r\n", Filepath);
        while(true);
    }

    /* -------------------------------------------------------------
       | 5. Get size of Kernel.elf                                 |                                    
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
       | 6. Allocate buffer for Kernel.elf                         |                                
       ------------------------------------------------------------- */

    Status = uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, FileInfo->FileSize, &FileBuffer);
    
    if (EFI_ERROR(Status)) {
        Print(L"Failed to allocate memory for %s.\r\n", Filepath);
        while(true);
    }

    /* -------------------------------------------------------------
       | 7. Read Kernel.elf into memory.                           |                              
       ------------------------------------------------------------- */

    UINTN FileSize = FileInfo->FileSize;
    Status = uefi_call_wrapper(File->Read, 3, File, &FileSize, FileBuffer);
    
    if (EFI_ERROR(Status)) {
        Print(L"Failed to read %s into memory.\r\n", Filepath);
        while(true);
    }

}

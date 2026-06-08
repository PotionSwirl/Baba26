#include <efi.h>
#include <efilib.h>
#include <stdbool.h>
#include "include/EFIUtils.h"

// We need this header for shim to let us execute.
/* It's doing us a favour after all - helping us clear pretty strict
   Secure Boot checks even when we're CLEARLY not a real OS! */

const char sbat_data[] __attribute__((section(".sbat"), used)) = 
    "sbat,1,SBAT Version,sbat,1,https://github.com\n"
    "Baba26,1,PotionSwirl,Baba26,1,https://github.com/PotionSwirl/Baba26\n\0";


EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {

    // Initialise UEFI libraries
    InitializeLib(ImageHandle, SystemTable);
    
    Print(L"Hi world!");

    while(true);

}

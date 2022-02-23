#include <efi.h>
#include <efilib.h>
 
void delay(int d) {
  for (int i = 0; i < d; i++) {
    for (int k = 0; k < 10000000; k++) {
      // Do nothing
    }
  } 
}

EFI_STATUS
EFIAPI
efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
  InitializeLib(ImageHandle, SystemTable);
  Print(L"Hello, why was this so hard?! T_T\n");
  for (int i = 0; i < 20; i++) {
    for (int j = 0; j < i+1; j++) {
      Print(L"*");
    }
    Print(L"\n");
    delay(100);
  }

  return EFI_SUCCESS;
}
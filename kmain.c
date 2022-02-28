#include <efi.h>
#include <efilib.h>

CHAR16* memTypeToStr(UINT32 type) {
	switch (type)
	{
		case EfiReservedMemoryType:
			return L"EfiReservedMemoryType";
		case EfiLoaderCode:
			return L"EfiLoaderCode";
		case EfiLoaderData:
			return L"EfiLoaderData";
		case EfiBootServicesCode:
			return L"EfiBootServicesCode";
		case EfiBootServicesData:
			return L"EfiBootServicesData";
		case EfiRuntimeServicesCode:
			return L"EfiRuntimeServicesCode";
		case EfiRuntimeServicesData:
			return L"EfiRuntimeServicesData";
		case EfiConventionalMemory:
			return L"EfiConventionalMemory";
		case EfiUnusableMemory:
			return L"EfiUnusableMemory";
		case EfiACPIReclaimMemory:
			return L"EfiACPIReclaimMemory";
		case EfiACPIMemoryNVS:
			return L"EfiACPIMemoryNVS";
		case EfiMemoryMappedIO:
			return L"EfiMemoryMappedIO";
		case EfiMemoryMappedIOPortSpace:
			return L"EfiMemoryMappedIOPortSpace";
		case EfiPalCode:
			return L"EfiPalCode";
		case EfiMaxMemoryType:
			return L"EfiMaxMemoryType";

	default:
		return L"UNKNOWN MEMORY TYPE";
	}
}

EFI_STATUS
EFIAPI
efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
	InitializeLib(ImageHandle, SystemTable);
	
	EFI_STATUS status;

	uefi_call_wrapper(SystemTable->ConOut->ClearScreen, 1, SystemTable->ConOut);
	uefi_call_wrapper(SystemTable->ConOut->SetAttribute, 2,
		SystemTable->ConOut,
		EFI_TEXT_ATTR(EFI_YELLOW, EFI_BLACK)
	);


	EFI_LOADED_IMAGE_PROTOCOL *imageProto;
	status = uefi_call_wrapper(SystemTable->BootServices->HandleProtocol, 3,
		ImageHandle,
		&gEfiLoadedImageProtocolGuid,
		&imageProto
	);
	if (status != EFI_SUCCESS) {
		Print(L"ERROR: Opening EFI_LOADED_IMAGE_PROTOCOL failed (*%d)\n", status);
		Input(L"", NULL, 1); 
		return status;
	}

	Print(L"Image loaded at addr 0x%x\n", imageProto->ImageBase);
	Print(L"Image size %dkB\n", imageProto->ImageSize/1000);
	Print(L"Image main at addr 0x%x\n", efi_main);

	Print(L"Firmware Info:\n");
	Print(L"    Vendor: %s\n", SystemTable->FirmwareVendor);

	uefi_call_wrapper(SystemTable->ConOut->OutputString, 2,
		SystemTable->ConOut,
		L"Arbitrary test (2)\r\n"
	);

	// Get memory map.
	VOID *mapBuffer;
	UINTN mapBufferInLen = 50*1024;
	UINTN mapBufferOutLen = mapBufferInLen;
	UINTN mapKey;
	UINTN descriptorSize;
	UINTN descriptorVersion; 
	uefi_call_wrapper(SystemTable->BootServices->AllocatePool, 2,
		EfiLoaderData,
		mapBufferInLen,
		&mapBuffer
	);
	uefi_call_wrapper(SystemTable->BootServices->GetMemoryMap, 5,
		&mapBufferOutLen,
		mapBuffer,
		&mapKey,
		&descriptorSize,
		&descriptorVersion
	);

	if (mapBufferOutLen > mapBufferInLen) {
		Print(L"ERROR: Call to 'GetMemoryMap' with too small of a buffer\n");
	} else {
		Print(L"\nMemory map:\n");
		Print(L"  buffer len: %dB\n", mapBufferOutLen);
		Print(L"  buffer ptr: 0x%x\n", mapBuffer);
		Print(L"  map key   : 0x%x\n", mapKey);
		Print(L"  desc size : %dB\n", descriptorSize);
		Print(L"  desc ver  : %d\n", descriptorVersion);
		Print(L"  num desc  : %d\n", mapBufferOutLen/descriptorSize);
		Input(L"", NULL, 1); 

		for (int i = 0; i < mapBufferOutLen; i += descriptorSize) {
			EFI_MEMORY_DESCRIPTOR desc = *(EFI_MEMORY_DESCRIPTOR*)(mapBuffer+i);
			Print(L"%3d)\n", i/descriptorSize);
			Print(L"    type        : %s (0x%x)\n", memTypeToStr(desc.Type), desc.Type);
			Print(L"    num pages   : %d\n", desc.NumberOfPages);
			Print(L"    padding     : %d\n", desc.Pad);
			Print(L"    attribute   : 0x%x\n", desc.Attribute);
			Print(L"    start (phys): 0x%x\n", desc.PhysicalStart);
			Print(L"    start (virt): 0x%x\n", desc.VirtualStart);


			Input(L"", NULL, 1); 
		}
	}

	/* DRAW GRAPHICS */
	Print(L"Doing graphics!\n");

	EFI_GRAPHICS_OUTPUT_PROTOCOL *graphicsProto;
	EFI_HANDLE *gopHandles;
	EFI_HANDLE gopHandle;
	UINTN gopHandlesLen;

	status = uefi_call_wrapper(SystemTable->BootServices->LocateHandleBuffer, 5,
		ByProtocol,
		&gEfiGraphicsOutputProtocolGuid,
		NULL,
		&gopHandlesLen,
		&gopHandles
	);
	if (status != EFI_SUCCESS) {
		Print(L"ERROR: Opening EFI_GRAPHICS_OUTPUT_PROTOCOL failed (*%d)\n", status);
		uefi_call_wrapper(SystemTable->BootServices->Stall, 1, 5*1000*1000);
		Input(L"", NULL, 1); 
		return status;
	}

	gopHandle = gopHandles[0];

	status = uefi_call_wrapper(SystemTable->BootServices->HandleProtocol, 3,
		gopHandle, // Use the first handle
		&gEfiGraphicsOutputProtocolGuid,
		&graphicsProto
	);
	if (status != EFI_SUCCESS) {
		Print(L"ERROR: Using HandleProtocol failed (*%d)\n", status);
		uefi_call_wrapper(SystemTable->BootServices->Stall, 1, 5*1000*1000);
		Input(L"", NULL, 1); 
		return status;
	}

	Print(L"Got %d graphics protocol handles\n", gopHandlesLen);

	// List graphics modes
	Print(L"\nAll (%d) graphics modes:\n", graphicsProto->Mode->MaxMode);
	for (UINT32 i = 0; i < graphicsProto->Mode->MaxMode; i++) {
		UINTN sizeOfInfo;
		EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
		status = uefi_call_wrapper(graphicsProto->QueryMode, 4,
			graphicsProto,
			i,
			&sizeOfInfo,
			&info
		);
		if (status != EFI_SUCCESS) {
			Print(L"ERROR: Using QueryMode failed (*%d)\n", status);
			uefi_call_wrapper(SystemTable->BootServices->Stall, 1, 5*1000*1000);
			Input(L"", NULL, 1); 
			return status;
		}

		Print(L"%3d)\n", i);
		Print(L"  resolution  : %dx%d\n", info->HorizontalResolution, info->VerticalResolution);
		Print(L"  pixel format: %d\n", info->PixelFormat);
		Print(L"  pix/scanline: %d\n", info->PixelsPerScanLine);
		Print(L"  pix. bitmask: %d\n", info->PixelInformation);
		Print(L"  version     : %d\n", info->Version);
		Input(L"", NULL, 1); 
	}

	status = uefi_call_wrapper(graphicsProto->SetMode, 2,
		graphicsProto,
		graphicsProto->Mode->MaxMode-1
	);
	if (status != EFI_SUCCESS) {
		Print(L"ERROR: Using SetMode failed (*%d)\n", status);
		uefi_call_wrapper(SystemTable->BootServices->Stall, 1, 5*1000*1000);
		Input(L"", NULL, 1); 
		return status;
	}

	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *gmi = graphicsProto->Mode->Info;
	Print(L"Current graphics mode\n");
	Print(L"  resolution  : %dx%d\n", gmi->HorizontalResolution, gmi->VerticalResolution);
	Print(L"  pixel format: %d\n", gmi->PixelFormat);
	Print(L"  pix/scanline: %d\n", gmi->PixelsPerScanLine);
	Print(L"  pix. bitmask: %d\n", gmi->PixelInformation);
	Print(L"  mod number  : %d\n", graphicsProto->Mode->Mode);
	Print(L"  version     : %d\n", gmi->Version);
	Input(L"", NULL, 1); 


	EFI_GRAPHICS_OUTPUT_BLT_PIXEL *rawfb =
		(EFI_GRAPHICS_OUTPUT_BLT_PIXEL*)graphicsProto->Mode->FrameBufferBase;
	INTN pixelSize;
	INTN totalPixels = gmi->VerticalResolution*gmi->PixelsPerScanLine;
	switch (gmi->PixelFormat) {
		case PixelBitMask:
			return 1;
		case PixelBlueGreenRedReserved8BitPerColor:
		case PixelRedGreenBlueReserved8BitPerColor:
			pixelSize =
			sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL);
			break;
	}

	int off = 1;
	int width = gmi->HorizontalResolution;
	int height = gmi->VerticalResolution;
	while (1) {
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				int clr = (x+off) * (y+off);
				rawfb[y*width+x] = *((EFI_GRAPHICS_OUTPUT_BLT_PIXEL*)&clr);
			}
		}
		off++;
		uefi_call_wrapper(SystemTable->BootServices->Stall, 1, 16*1000);
	}


	uefi_call_wrapper(SystemTable->BootServices->Stall, 1, 30*1000*1000);
	Print(L"\nReturning in 30 seconds…\n");

	return EFI_SUCCESS;
}
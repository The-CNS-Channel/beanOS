#include "lib.h"

#define SIMPLE_READ_SIGNATURE       EFI_SIGNATURE_32('s','r','d','r')
typedef struct _SIMPLE_READ_FILE {
    UINTN               Signature;
    BOOLEAN             FreeBuffer;
    VOID                *Source;
    UINTN               SourceSize;
    EFI_FILE_HANDLE     FileHandle;
} SIMPLE_READ_HANDLE;

       

EFI_STATUS
OpenSimpleReadFile (
    IN BOOLEAN                  BootPolicy,
    IN VOID                     *SourceBuffer   OPTIONAL,
    IN UINTN                    SourceSize,
    IN OUT EFI_DEVICE_PATH      **FilePath,
    OUT EFI_HANDLE              *DeviceHandle,
    OUT SIMPLE_READ_FILE        *SimpleReadHandle
    )

{
    SIMPLE_READ_HANDLE          *FHand;
    EFI_DEVICE_PATH             *UserFilePath;
    EFI_DEVICE_PATH             *TempFilePath;
    EFI_DEVICE_PATH             *TempFilePathPtr;
    FILEPATH_DEVICE_PATH        *FilePathNode;
    EFI_FILE_HANDLE             FileHandle, LastHandle;
    EFI_STATUS                  Status;
    EFI_LOAD_FILE_INTERFACE     *LoadFile;
  
    FHand = NULL;
    UserFilePath = *FilePath;


    FHand = AllocateZeroPool (sizeof(SIMPLE_READ_HANDLE));
    if (!FHand) {
        Status = EFI_OUT_OF_RESOURCES;
        goto Done;
    }

    *SimpleReadHandle = (SIMPLE_READ_FILE) FHand;
    FHand->Signature = SIMPLE_READ_SIGNATURE;


    if (SourceBuffer) {
        FHand->Source = SourceBuffer;
        FHand->SourceSize = SourceSize;
        *DeviceHandle = NULL;
        Status = EFI_SUCCESS;
        goto Done;
    } 


    FileHandle = NULL;
    Status = uefi_call_wrapper(BS->LocateDevicePath, 3, &FileSystemProtocol, FilePath, DeviceHandle);
    if (!EFI_ERROR(Status)) {
        FileHandle = LibOpenRoot (*DeviceHandle);
    }

    Status = FileHandle ? EFI_SUCCESS : EFI_UNSUPPORTED;


    FilePathNode = (FILEPATH_DEVICE_PATH *) *FilePath;
    while (!IsDevicePathEnd(&FilePathNode->Header)) {


        if (DevicePathType(&FilePathNode->Header) != MEDIA_DEVICE_PATH ||
            DevicePathSubType(&FilePathNode->Header) != MEDIA_FILEPATH_DP) {
            Status = EFI_UNSUPPORTED;
        }


        if (EFI_ERROR(Status)) {
            break;
        }
        

        LastHandle = FileHandle;
        FileHandle = NULL;

        Status = uefi_call_wrapper(
			LastHandle->Open,
			5, 
                        LastHandle,
                        &FileHandle,
                        FilePathNode->PathName,
                        EFI_FILE_MODE_READ,
                        0
                        );
        
        
        uefi_call_wrapper(LastHandle->Close, 1, LastHandle);


        FilePathNode = (FILEPATH_DEVICE_PATH *) NextDevicePathNode(&FilePathNode->Header);
    }


    if (!EFI_ERROR(Status)) {
        ASSERT(FileHandle);
        FHand->FileHandle = FileHandle;
        goto Done;
    }


    if (FileHandle) {
        uefi_call_wrapper(FileHandle->Close, 1, FileHandle);
        FileHandle = NULL;
        *FilePath = UserFilePath;
    }


    if (Status != EFI_UNSUPPORTED) {
        goto Done;
    }


    Status = LibDevicePathToInterface (&LoadFileProtocol, *FilePath, (VOID*)&LoadFile);
    if (!EFI_ERROR(Status)) {

        TempFilePath = DuplicateDevicePath (*FilePath);

        TempFilePathPtr = TempFilePath;

        Status = uefi_call_wrapper(BS->LocateDevicePath, 3, &LoadFileProtocol, &TempFilePath, DeviceHandle);

        FreePool (TempFilePathPtr);


        SourceSize = 0;
        Status = uefi_call_wrapper(
		    LoadFile->LoadFile,
			5,
                    LoadFile,
                    *FilePath,
                    BootPolicy,
                    &SourceSize,
                    NULL
                    );



        if (Status == EFI_BUFFER_TOO_SMALL) {
            SourceBuffer = AllocatePool (SourceSize);
            
            if (SourceBuffer) {
                FHand->FreeBuffer = TRUE;
                FHand->Source = SourceBuffer;
                FHand->SourceSize = SourceSize;

                Status = uefi_call_wrapper(
			    LoadFile->LoadFile,
				5,
                            LoadFile,
                            *FilePath,
                            BootPolicy,
                            &SourceSize,
                            SourceBuffer
                            );  
            }
        }


        if (!EFI_ERROR(Status) || Status == EFI_ALREADY_STARTED) {
            goto Done;
        }
    }


    DEBUG ((D_LOAD|D_WARN, "OpenSimpleReadFile: Device did not support a known load protocol\n"));
    Status = EFI_UNSUPPORTED;

Done:

    if (EFI_ERROR(Status) && (Status != EFI_ALREADY_STARTED)) {
        if (FHand) {
            if (FHand->FreeBuffer) {
                FreePool (FHand->Source);
            }

            FreePool (FHand);
        }
    }

    return Status;
}

EFI_STATUS
ReadSimpleReadFile (
    IN SIMPLE_READ_FILE     UserHandle,
    IN UINTN                Offset,
    IN OUT UINTN            *ReadSize,
    OUT VOID                *Buffer
    )
{
    UINTN                   EndPos;
    SIMPLE_READ_HANDLE      *FHand;
    EFI_STATUS              Status;

    FHand = UserHandle;
    ASSERT (FHand->Signature == SIMPLE_READ_SIGNATURE);
    if (FHand->Source) {


        EndPos = Offset + *ReadSize;
        if (EndPos > FHand->SourceSize) {
            *ReadSize = FHand->SourceSize - Offset;
            if (Offset >= FHand->SourceSize) {
                *ReadSize = 0;
            }
        }

        CopyMem (Buffer, (CHAR8 *) FHand->Source + Offset, *ReadSize);
        Status = EFI_SUCCESS;

    } else {


        Status = uefi_call_wrapper(FHand->FileHandle->SetPosition, 2, FHand->FileHandle, Offset);

        if (!EFI_ERROR(Status)) {
            Status = uefi_call_wrapper(FHand->FileHandle->Read, 3, FHand->FileHandle, ReadSize, Buffer);
        }
    }

    return Status;
}


VOID
CloseSimpleReadFile (
    IN SIMPLE_READ_FILE     UserHandle
    )
{
    SIMPLE_READ_HANDLE      *FHand;

    FHand = UserHandle;
    ASSERT (FHand->Signature == SIMPLE_READ_SIGNATURE);


    if (FHand->FileHandle) {
        uefi_call_wrapper(FHand->FileHandle->Close, 1, FHand->FileHandle);
    }


    if (FHand->FreeBuffer) {
        FreePool (FHand->Source);
    }


    FreePool (FHand);
}

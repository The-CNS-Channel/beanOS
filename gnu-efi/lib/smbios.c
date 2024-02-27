#include "lib.h"

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
#endif

EFI_STATUS
LibGetSmbiosSystemGuidAndSerialNumber (
    IN  EFI_GUID    *SystemGuid,
    OUT CHAR8       **SystemSerialNumber
    )
{
    EFI_STATUS                  Status;
    SMBIOS_STRUCTURE_TABLE      *SmbiosTable;
    SMBIOS_STRUCTURE_POINTER    Smbios;  
    SMBIOS_STRUCTURE_POINTER    SmbiosEnd;  
    UINT16                      Index;
    
    Status = LibGetSystemConfigurationTable(&SMBIOSTableGuid, (VOID**)&SmbiosTable);
    if (EFI_ERROR(Status)) {
        return EFI_NOT_FOUND;
    }

    Smbios.Hdr = (SMBIOS_HEADER *)SmbiosTable->TableAddress;
    SmbiosEnd.Raw = (UINT8 *)(SmbiosTable->TableAddress + SmbiosTable->TableLength);
    for (Index = 0; Index < SmbiosTable->TableLength ; Index++) {
        if (Smbios.Hdr->Type == 1) {
            if (Smbios.Hdr->Length < 0x19) {
                continue;
            }

            
            CopyMem (SystemGuid, &Smbios.Type1->Uuid, sizeof(EFI_GUID));
            *SystemSerialNumber = LibGetSmbiosString(&Smbios, Smbios.Type1->SerialNumber);
            return EFI_SUCCESS;
        }

        LibGetSmbiosString (&Smbios, -1);

        if (Smbios.Raw >= SmbiosEnd.Raw) {

            return EFI_SUCCESS;
        }
    }

    return EFI_SUCCESS;
}

CHAR8*
LibGetSmbiosString (
    IN  SMBIOS_STRUCTURE_POINTER    *Smbios,
    IN  UINT16                      StringNumber
    )

{
    UINT16  Index;
    CHAR8   *String;


    String = (CHAR8 *)(Smbios->Raw + Smbios->Hdr->Length);

    for (Index = 1; Index <= StringNumber; Index++) {
        if (StringNumber == Index) {
            return String;
        }


        for (; *String != 0; String++);
        String++;

        if (*String == 0) {
			
            Smbios->Raw = (UINT8 *)++String;
            return NULL;        
        }
    }
    return NULL;        
}

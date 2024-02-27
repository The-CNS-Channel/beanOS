#include "lib.h"


INTN
StrCmp (
    IN CONST CHAR16   *s1,
    IN CONST CHAR16   *s2
    )
{
    return RtStrCmp(s1, s2);
}

INTN
StrnCmp (
    IN CONST CHAR16   *s1,
    IN CONST CHAR16   *s2,
    IN UINTN    len
    )
{
    while (*s1  &&  len) {
        if (*s1 != *s2) {
            break;
        }

        s1  += 1;
        s2  += 1;
        len -= 1;
    }

    return len ? *s1 - *s2 : 0;
}


INTN EFIAPI
LibStubStriCmp (
    IN EFI_UNICODE_COLLATION_INTERFACE  *This EFI_UNUSED,
    IN CHAR16                           *s1,
    IN CHAR16                           *s2
    )
{
    return StrCmp (s1, s2);
}

VOID EFIAPI
LibStubStrLwrUpr (
    IN EFI_UNICODE_COLLATION_INTERFACE  *This EFI_UNUSED,
    IN CHAR16                           *Str EFI_UNUSED
    )
{
}

INTN
StriCmp (
    IN CONST CHAR16   *s1,
    IN CONST CHAR16   *s2
    )
{
    if (UnicodeInterface == &LibStubUnicodeInterface)
    	return UnicodeInterface->StriColl(UnicodeInterface, (CHAR16 *)s1, (CHAR16 *)s2);
    else
	return uefi_call_wrapper(UnicodeInterface->StriColl, 3, UnicodeInterface, (CHAR16 *)s1, (CHAR16 *)s2);
}

VOID
StrLwr (
    IN CHAR16   *Str
    )
{
    if (UnicodeInterface == &LibStubUnicodeInterface)
    	UnicodeInterface->StrLwr(UnicodeInterface, Str);
    else uefi_call_wrapper(UnicodeInterface->StrLwr, 2, UnicodeInterface, Str);
}

VOID
StrUpr (
    IN CHAR16   *Str
    )
{
    if (UnicodeInterface == &LibStubUnicodeInterface)
        UnicodeInterface->StrUpr(UnicodeInterface, Str);
    else uefi_call_wrapper(UnicodeInterface->StrUpr, 2, UnicodeInterface, Str);
}

VOID
StrCpy (
    IN CHAR16   *Dest,
    IN CONST CHAR16   *Src
    )
{
    RtStrCpy (Dest, Src);
}

VOID
StrnCpy (
    IN CHAR16   *Dest,
    IN CONST CHAR16   *Src,
    IN UINTN     Len
    )
{
    RtStrnCpy (Dest, Src, Len);
}

CHAR16 *
StpCpy (
    IN CHAR16   *Dest,
    IN CONST CHAR16   *Src
    )
{
    return RtStpCpy (Dest, Src);
}

CHAR16 *
StpnCpy (
    IN CHAR16   *Dest,
    IN CONST CHAR16   *Src,
    IN UINTN     Len
    )
{
    return RtStpnCpy (Dest, Src, Len);
}

VOID
StrCat (
    IN CHAR16   *Dest,
    IN CONST CHAR16   *Src
    )
{
    RtStrCat(Dest, Src);
}

VOID
StrnCat (
    IN CHAR16   *Dest,
    IN CONST CHAR16   *Src,
    IN UINTN     Len
    )
{
    RtStrnCat(Dest, Src, Len);
}


UINTN
StrnLen (
    IN CONST CHAR16   *s1,
    IN UINTN           Len
    )
{
    return RtStrnLen(s1, Len);
}

UINTN
StrLen (
    IN CONST CHAR16   *s1
    )
{
    return RtStrLen(s1);
}

UINTN
StrSize (
    IN CONST CHAR16   *s1
    )
{
    return RtStrSize(s1);
}

CHAR16 *
StrDuplicate (
    IN CONST CHAR16   *Src
    )
{
    CHAR16      *Dest;
    UINTN       Size;

    Size = StrSize(Src);
    Dest = AllocatePool (Size);
    if (Dest) {
        CopyMem (Dest, Src, Size);
    }
    return Dest;
}

UINTN
strlena (
    IN CONST CHAR8    *s1
    )
{
    UINTN        len;

    for (len=0; *s1; s1+=1, len+=1) ;
    return len;
}

UINTN
strcmpa (
    IN CONST CHAR8    *s1,
    IN CONST CHAR8    *s2
    )
{
    while (*s1) {
        if (*s1 != *s2) {
            break;
        }

        s1 += 1;
        s2 += 1;
    }

    return *s1 - *s2;
}

UINTN
strncmpa (
    IN CONST CHAR8    *s1,
    IN CONST CHAR8    *s2,
    IN UINTN    len
    )
{
    while (*s1  &&  len) {
        if (*s1 != *s2) {
            break;
        }

        s1  += 1;
        s2  += 1;
        len -= 1;
    }

    return len ? *s1 - *s2 : 0;
}



UINTN
xtoi (
    CONST CHAR16  *str
    )
{
    UINTN       u;
    CHAR16      c;

    while (*str && *str == ' ') {
        str += 1;
    }

    u = 0;
    while ((c = *(str++))) {
        if (c >= 'a'  &&  c <= 'f') {
            c -= 'a' - 'A';
        }

        if ((c >= '0'  &&  c <= '9')  ||  (c >= 'A'  &&  c <= 'F')) {
            u = (u << 4)  |  (c - (c >= 'A' ? 'A'-10 : '0'));
        } else {
            break;
        }
    }

    return u;
}

UINTN
Atoi (
    CONST CHAR16  *str
    )
{
    UINTN       u;
    CHAR16      c;

    while (*str && *str == ' ') {
        str += 1;
    }

    u = 0;
    while ((c = *(str++))) {
        if (c >= '0' && c <= '9') {
            u = (u * 10) + c - '0';
        } else {
            break;
        }
    }

    return u;
}

BOOLEAN
MetaMatch (
    IN CHAR16   *String,
    IN CHAR16   *Pattern
    )
{
    CHAR16  c, p, l;

    for (; ;) {
        p = *Pattern;
        Pattern += 1;

        switch (p) {
        case 0:
            return *String ? FALSE : TRUE;

        case '*':
            while (*String) {
                if (MetaMatch (String, Pattern)) {
                    return TRUE;
                }
                String += 1;
            }
            return MetaMatch (String, Pattern);

        case '?':
            if (!*String) {
                return FALSE;
            }
            String += 1;
            break;

        case '[':
            c = *String;
            if (!c) {
                return FALSE;
            }

            l = 0;
            while ((p = *Pattern++)) {
                if (p == ']') {
                    return FALSE;
                }

                if (p == '-') {
                    p = *Pattern;
                    if (p == 0 || p == ']') {
                        return FALSE;
                    }

                    if (c >= l && c <= p) {
                        break;
                    }
                }

                l = p;
                if (c == p) {
                    break;
                }
            }

            while (p && p != ']') {
                p = *Pattern;
                Pattern += 1;
            }

            String += 1;
            break;

        default:
            c = *String;
            if (c != p) {
                return FALSE;
            }

            String += 1;
            break;
        }
    }
}


BOOLEAN EFIAPI
LibStubMetaiMatch (
    IN EFI_UNICODE_COLLATION_INTERFACE  *This EFI_UNUSED,
    IN CHAR16                           *String,
    IN CHAR16                           *Pattern
    )
{
    return MetaMatch (String, Pattern);
}


BOOLEAN
MetaiMatch (
    IN CHAR16   *String,
    IN CHAR16   *Pattern
    )
{
    if (UnicodeInterface == &LibStubUnicodeInterface)
    	return UnicodeInterface->MetaiMatch(UnicodeInterface, String, Pattern);
    else return uefi_call_wrapper(UnicodeInterface->MetaiMatch, 3, UnicodeInterface, String, Pattern);
}

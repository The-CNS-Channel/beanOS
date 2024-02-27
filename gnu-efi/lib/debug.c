#include "lib.h"

INTN
DbgAssert (
    IN CONST CHAR8    *FileName,
    IN INTN           LineNo,
    IN CONST CHAR8    *Description
    )
{
    DbgPrint (D_ERROR, (CHAR8 *)"%EASSERT FAILED: %a(%d): %a%N\n", FileName, LineNo, Description);

    BREAKPOINT();
    return 0;
}


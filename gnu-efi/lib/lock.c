#include "lib.h"


VOID
InitializeLock (
    IN OUT FLOCK    *Lock,
    IN EFI_TPL      Priority
    )
{
    Lock->Tpl = Priority;
    Lock->OwnerTpl = 0;
    Lock->Lock = 0;
}


VOID
AcquireLock (
    IN FLOCK    *Lock
    )
{
    RtAcquireLock (Lock);
}


VOID
ReleaseLock (
    IN FLOCK    *Lock
    )
{
    RtReleaseLock (Lock);
}

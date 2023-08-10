#include <Memory/Memory.h>
#include <Scheduler/Sync.h>

LOCK mm_lock;

// TODO use this for all the API calls.
VOID AcquireMmLock()
{}

BOOL KERNEL_ASYNC MmReentStat(VOID)
{
    return MutexWasLocked(&mm_lock);
}

// Get size of block?
U32 MemInfo(U8 op)
{}

// Order of initialization:
// - V86 must be ready before MM
// - Chain must be initialized first
// - Map initializes new kernel page tables and allocates for userspace VAS
//
// end_of_kernel_physical: Beginning of free extended memory.
//
VOID InitMM(PVOID end_of_kernel_physical)
{
    ChainInit();
}

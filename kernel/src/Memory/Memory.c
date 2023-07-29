#include <Scheduler/Sync.h>

LOCK mm_lock;

VOID AcquireMmLock()
{}

BOOL KERNEL_ASYNC MmReentStat(VOID)
{
    return MutexWasLocked(&mm_lock);
}

U32 MemInfo(U8 op)
{}

VOID InitMM()
{}

#include <Memory/Core.h>
#include <Scheduler/Sync.h>

ATOMIC mm_lock;

// TODO use this for all the API calls.
VOID Acquire_MM_Lock()
{}

kernel_async BOOL MM_Reent_Stat(VOID)
{
    return MutexWasLocked(&mm_lock);
}

// Get size of block?
U32 Mem_Info(U8 op)
{}

// Order of initialization:
// - V86 must be ready before MM
// - Chain must be initialized first
// - Map initializes new kernel page tables and allocates for userspace VAS
//
// end_of_kernel_physical: Beginning of free extended memory.
//
VOID Init_MM(PVOID end_of_kernel_physical)
{
    ChainInit();
}

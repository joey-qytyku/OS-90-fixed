#include <Scheduler.h>
#include <Memory.h>

DWORD MmAllocateBlock(HANDLE heap_hnd, DWORD bytes);

VOID MmDeleteBlock();

PVOID MmFreezeMem(HANDLE heap_hnd, HANDLE block_hnd);
VOID MmReleaseMem(HANDLE heap_hnd, HANDLE block_hnd);

HANDLE MmCreateHeap(PVOID virtual_addr);

//
// Note: NOT THREAD SAFE. Do not execute in an ISR.
// If fails, segment returned is FFFF, or -1
//
WORD MmAllocateDosMemory(WORD paragraphs)
{
    DWORD regparm[RD_NUM_DWORDS];

    regparm[RD_EAX] = 0x4800;
    regparm[RD_EBX] = paragraphs;

    // INIT TRAP FRAME!

    ScVirtual86_Int(regparm, 0x21);

    if (regparm[RD_EFLAGS] & 1)
    {
        // If the DOS call fails, it returns the maximum block size
        // with available memory. This function assumes the caller wants that
        // exact amount of memory
        return 0xFFFF;  // Error
    }
    return regparm[RD_EAX];
}

VOID MmFreeDosMemory(WORD segment)
{
}

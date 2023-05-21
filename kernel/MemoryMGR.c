#include <Scheduler.h>
#include <Memory.h>

DWORD MmAllocateBlock(HANDLE heap_hnd, DWORD bytes);

VOID MmDeleteBlock();

// BRIEF:
//      Allocate memory in the conventional memory region.
// RETURN:
//      A 32-bit pointer to the location. If it fails, this is NULL.
//      In DOS, the allocate function returns the maximum size of a block if
//      it cannot allocate all of it. Here, we use a separate call for that.
//
//      Only allocate DOS memory unless data must be exchanged underneath 1M.
//      For example, ISA DMA and API call translation.
//
PVOID AllocDosMem(WORD pgr)
{
}

VOID MmResizeDosMem(WORD segment)
{
}

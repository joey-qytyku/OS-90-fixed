# Memory Mapping (TODO)

OS/90 preallocates all page tables of the virtual address space and never extends it.

Remapping memory takes a subchain range to perform the mapping on so that dynamic structures can be efficient. It is not necessary to change the base address being used.
Listing:

- PVOID M_Map(PVOID base, LONG chain, LONG attr)
- SIGLONG M_ChainRemap(PVOID baseaddr, LONG chain, LONG start, LONG len, LONG attr)
- VOID M_MapPhysToVirt(PVOID phys, PVOID virt, LONG bytes)
- PVOID M_ReserveMapping(LONG bytes)
- SIGLONG M_ReleaseMapping(LONG bytes)
- VOID M_SetAttr(PVOID addr, LONG bytes, LONG attr)
- VOID M_FlushTLB(PVOID addr, LONG bytes);

## M_Map

Maps a block of memory to a virtual address. Generates uncommitted pages as necessary. Only the necessary number of pages are reserved, so remapping is necessary if the chain grows in size.

Using the first 1MB+64K is always invalid since OS/90 does not page that memory.

## M_ChainRemap

Remap a chain to the exact same location but with a different size. If there is a collision with in-use virtual addresses this function will do nothing and fail.

The base and length of the region to remap is specified for creater efficiency.

## M_MapPhysToVirt

Map a physical address range to a virtual one. This is required for accessing MMIO devices. Ensure that the range has been allocated before calling this function.

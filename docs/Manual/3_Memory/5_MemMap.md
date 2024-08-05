# Memory Mapping (TODO)

OS/90 preallocates all page tables of the virtual address space and never extends it.

Remapping memory takes a subchain range to perform the mapping on so that dynamic structures can be efficient. It is not necessary to change the base address being used.
Listing:

- STAT M_Map(LONG chain, PVOID baseaddr, LONG start, LONG len, LONG attr_override)
- VOID M_MapPhysToVirt(PVOID phys, PVOID virt, LONG bytes)
- PVOID M_ReserveMapping(LONG bytes)
- STAT M_ReleaseMapping(PVOID addr)
- VOID M_SetFlags(PVOID addr, LONG bytes, LONG pflags)
- VOID M_FlushTLB(PVOID addr, LONG bytes);

## M_Map

> Is this function too complicated? Launch an inquiry.

This is the memory mapping Swiss army knife. It can handle dynamic structures, partial mapping, and other things.

Maps a chain to an address, which should have been allocated by reserving a mapping. This is suitable for remapping the same chain.

The present bit is ignored as usual.

## M_MapPhysToVirt

Map a physical address range to a virtual one. This is required for accessing MMIO devices. Ensure that the range has been allocated before calling this function.

## M_VirtualAlloc

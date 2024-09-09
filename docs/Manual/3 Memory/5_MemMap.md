## Memory Mapping (TODO)

OS/90 preallocates all page tables of the virtual address space and never extends it.

Remapping memory takes a subchain range to perform the mapping on so that dynamic structures can be efficient. It is not necessary to change the base address being used.

This API is used to implement the more complex memory management features.

Listing:

```
STAT M_Map(
	LONG    chain,
	PVOID   base_addr,
	LONG    start,
	LONG    len,
	LONG    page_flags);

VOID M_MapPhysToVirt(
	PVOID phys,
	PVOID virt,
	LONG bytes)

PVOID M_ReserveMapping(LONG bytes);

STAT M_ReleaseMapping(PVOID addr);

VOID M_VmCopy(
	PVOID   va_to,
	PVOID   va_from,
	LONG    bytes);

VOID M_SetFlags(
	PVOID   addr,
	LONG    bytes,
	LONG    pflags);

VOID M_FlushTLB(
	PVOID   addr,
	LONG    bytes);
```

### PVOID M_ReserveMapping(LONG bytes)

Reserve a range of addresses in the virtual address space. They are zeroed out by default and accessing is always an error until later mapped.

This will not refresh the TLB.

### STAT M_Map(LONG chain, LONG page_flags)

Maps a chain to an address, which should have been allocated by reserving a mapping. This is suitable for remapping the same chain.

The present bit is ignored as usual.

The TLB will be refreshed using the optimal method for the CPU.

### STAT M_Uncommit(PVOID addr, LONG bytes)

Declare a range of pages as uncommitted, deferring allocation upon access.

### M_MapPhysToVirt

Map a physical address range to a virtual one. This is required for accessing MMIO devices. Ensure that the range has been allocated before calling this function.

### VOID M_VmCopy(PVOID va_to, PVOID va_from, LONG bytes)

Copy a virtual address space from one location to another. Essentially copies page table entries.

Inputs are not checked. Do not copy anywhere not previously reserved. The purpose of this function is to support dynamic data structures that require a growing address space.

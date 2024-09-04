# Memory Allocator

The kernel heap allocator uses a pool the size of about 1% of the extended memory. The entire region is pagable, writable, cache enabled, and never resizes.

An alignment of 16 is guaranteed for all blocks.

All allocation failures must be non-critical.

## Master Heap Table (MHT)

typedef struct {
	// The largest block that can be allocated.
	SHORT   largest_block;

	// The number of unallocated chunks
	LONG    chucks_fragments;

	// The size of the heap including all chunks
	LONG    chunks_total;

	// Pointer to MPT
	P_MPE   mpt;
}MHT;

Chunks are 16-byte.

## Master Pointer Table

The master pointer table contains a pointer to every allocation. Each entry is directly dereferenciable.

An entry is eight bytes and uses this structure:
```
typedef struct {
	PVOID   pointer;
	BYTE    lock;
	BYTE    free_chunks_after;
	BYTE    alloc_size;
}MPE;
```

Each MPE contains a count of free chunks after the allocated region (including sentinel bytes).


## Defragmentation Strategy

Fragmentation is not a problem until memory is low. There is no need to defragment except in cases where a memory request cannot be fullfilled.

The allocator uses attempts to find the best possible fit at all times.

## Heap Validation

The heap validator function checks the following:
- Master pointer table must always point to data in the pool.
- Lock count is not overlocked
- Sentinel bytes in the allocation are not overwritten
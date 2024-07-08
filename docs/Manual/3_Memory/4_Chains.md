# Chain Management
The OS/90 memory manager uses a FAT-style linked list table to allocate pages. An allocation is called a chain, and a chain ID is the index to the first block in the chain. Chain IDs are used throughout the chain allocation API.

The very last entry in a chain holds a count of how many uncommitted pages are to be mapped when mapping takes place. These do NOT get memory block entries.
Modifications to chains do NOT change mappings whatsoever.

Each page frame represented in the table has a persistent page bit state, which includes page modifiers. Page mods specified explicitly as acceptable for driver use in the previous chapter may be used.

- LONG M_Alloc(LONG bytes_commit, LONG bytes_uncommit, LONG map_with)
- STAT M_Free(LONG chain)
- STAT M_ResizeWithCommit(LONG chain, SIGLONG delta_bytes)
- STAT M_ExtendUncommit(LONG chain, LONG bytes)

## M_Alloc

Allocates a chain with `bytes_commit` bytes rounded to a page count and specifies the very last entry the number of uncommitted blocks that must later be mapped.

`bytes_uncommit` is the number of bytes rounded to a page count that come directly above the committed region after allocation. Note that this happens after the committed pages are allocated.

Consider this example: M_Alloc(1, 1, NULL). The committed pages are allocated first in the chain, and then the uncommitted pages are memorized.  4096 bytes are committed and 4096 bytes are not.
`bytes_commit` may never be zero.

## M_Free

This will delete the chain completely. No PAGE_PROC's are used if this is called. Ensure that the virtual address space is deallocated before using.

M_Free can fail. If the chain ID does not reference the start of the chain, an error will occur.

## M_ResizeWithCommit

Extends the chain and cuts into the uncommitted region if necessary. Does NOT remap.

## M_ExtendUncommit

Extends the chain and does not alter the number of uncommitted pages.

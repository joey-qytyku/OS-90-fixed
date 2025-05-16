# malloc

OS/90 uses the same malloc for user and kernel. It is C standards compliant.

## Bit Scan Optimzation

A lookup table is used instead of running the x86 bit scan instruction or inline equivalent. A bit scan instruction can take up to 100 clocks on 386, 486, and 586.

This would be totally unthinkable on a modern CPU as a bit scan can be done in few clock cycles and a cache miss could make it pointless.

This involves branching but is still much faster than bit scanning.

## Arenas

Memory is allocated in arenas that are made of frames. Each arena has a header that maintains which chunks are allocated.

The header contains two pointers for linked lists and some number of bytes for a bit map. It is always 16 bytes long. This implies 16-byte alignment for all allocations.

The following are the properties of each arena:
- 51 chunks of 80
- 17 chunks of 240
- 8  chunks of 510
- 4  chunks of 1020
- 2  chunks of 2040

4 bytes are excluded for sentinel words.

Anything too large for the highest granularity is allocated as pages. We can tell if the allocation is page granular by the fact that it is aligned to a page boundary. Otherwise, we are freeing something else.

## Implementation Layout

Single procedures are used to:
- Construct a new frame
- Allocate memory

The number of bytes to bit scan is also taken as an argument to the basic function.

void *malloc_base(, long long max_on_mask)

## Lazy Free

Do I need this?

## Garbage Collection

This is highly critical to avoid running out of memory.

When a frame is fully free, it could be retained and used for reallcocations, but this will leak until the system crashes.

When memory is freed, the frame is also freed if it has no entries.

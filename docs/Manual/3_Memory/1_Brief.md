# Memory Manager Brief

OS/90 handles memory using a page size of 4K. There are no plans to support transparent hugepages like on Linux, but such a feature is possible since all operations use bytes and are rounded to pages.

The physical memory is mapped in a static array in the Physical Block Table, where each entry represents a page frame at a specific address.

All memory is locked by default. Memory must specifically be marked as transient in order to be ellegible for swapping.

OS/90 is built on the idea of "unused RAM is wasted RAM" and the memory manager reflects this intent.

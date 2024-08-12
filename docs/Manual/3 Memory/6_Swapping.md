# Virtual Memory

OS/90 is intended to run optimimally on low memory systems. It implements demand paging.

Disk drivers are required to cooperate with the kernel for the sole purpose of swapping. Disk transfers must happen without caching whatsoever.

## The Swap File

There is one swap file called `_SWP_.___` and it must be on the boot disk.

A tool called MKSWAP in the OS90 directory does this by getting the free space left, creating a file exactly that large, and then resizing down to the desired size. The system attribute is used to indicate that the file cannot be physically moved or deleted.

The drive should be defragmented before making the swap file to avoid fragmenting further.

> Tructation on DOS happens like this: (https://stackoverflow.com/questions/2869050/shrinking-or-partially-truncating-a-file-in-dos-fat). Basically, by writing with zero length.

## Demand Paging

### The Demand Paging Buffer

A demand page buffer is allocated of at least 2 pages or 2 disk sectors, whichever is larger, is used to keep the absolute minimum of the paged out data in memory.

The reason why it works like this is to permit non-aligned access to pages and the same for a disk. For example, if 0x6004FFF is accessed as a word, it will cross a page boundary. This requires reading enough data and mapping the right number of pages.

The buffer can only be controlled by one task at a time. If another task needs to demand page, it will spin on the MM lock.

### Operation

Swap space is allocatable as physical memory is. Swap pages can be mapped to any reserved address space and any pages can be copied from the memory to the transfer buffer to remove it from memory.

### Implementation Interface

> What is this? https://stanislavs.org/helppc/int_21-33.html

See the header `MMSWAP.H`.

### API Listing

```
LONG V_SwapStat(LONG code);

LONG V_SwapReserve(LONG bytes)
```

### LONG V_SwapReserve(LONG bytes)

Reserves a range of pages in the swap file.

### STAT V_EvictPages(PVOID vaddr, LONG bytes)

Maybe I can do something advanced? Have a high-level "region" structure with a present memory quota for transfer buffering?

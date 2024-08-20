## Virtual Memory

OS/90 supports demand paged memory. Using zone allocation, swap space can be allocated like RAM and data can be copied to it as well.

Swap-related functions use page granularity rather than bytes, as there is little need for the conversion.

### API Listing

The API contains a number of functions mostly used internally. Because page faults are esentially system calls in OS/90, the API can be called at any point in time.

Each swap-related function must be called in T1 or T2. They will all hold the MM lock.

```
typedef struct {
	LONG    swap_pages;
	LONG    swap_pages_used;
	LONG    pad[4];
}VMSTAT,*PVMSTAT;

VOID M_VmStat(PVMSTAT v);
```

```
PZENT M_SwapAlloc(LONG pages)
STAT M_SwapFree(PZENT alloc, LONG pages);
VOID M_SwapWrite(
	PZENT   chain,
	PVOID   buff,
	LONG    pages
);

VOID M_SwapRead(PZENT chain, PVOID buff, LONG pages);

STAT M_SwapPageout();
STAT M_SwapPagein();

VOID M_MarkCandidate(PVOID va, LONG bytes);

LONG AcquireDemandBuffer(VOID);

PVOID M_VirtualAlloc(
	LONG    bytes,
	LONG    op_flags,
	PVOID   req_base
);
```

### STAT M_SwapAlloc(LONG pages)

Allocate pages on the swap file. Works the same as Z_Alloc but is page granular.

### M_SwapFree(LONG pages)

Free pages in swap.

### M_SwapWrite(PZENT chain, PVOID buff, LONG pages)

Copy pages into the swap space. The `data` and `buffer` must be page aligned and cannot overlap.

The data will be written to the swap pages indicated by the chain.

> The buffer must be locked and page aligned. `chain` must not be NULL.

### M_SwapRead(PZENT chain, PVOID buff, LONG pages)

Read from the swap space into memory.

> Same preconditions as M_SwapWrite.

### STAT M_SwapPageout()

### LONG M_AcquireDemandBuffer(VOID)

Acquire the demand paging buffer for the active thread. Spins until it is available and returns the number of pages that can be put into it.

This will flush whatever is inside it back to the disk. There is no release operation.

> Drivers should not use this. The OS uses it and M_SwapRead to implement demand paging.

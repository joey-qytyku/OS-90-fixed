# Memory Manager

The OS/90 memory manager supports:
- Demand paging (e.g. loading a program larger than RAM)
- Multi-granularity zone-based page frame allocations
- ISA DMA buffer allocation
- Single address space
- Uncommitted memory

RAM and page file space are indexed in an array called the MAT or Memory Allocation Table.

If there is no more memory, demand-paged space is allocated. A demand paging window is used to achieve orthogonality.

> Swap is an incorrect term. OS/90 does not swap.

## LONG M_GetInfo(BYTE type)

```
0 - Free extended memory pages
1 - Size of extended memory

2 - Free disk backed space in pages (DBS)
3 - Size of DBS in pages

4 - Address space pages free
5 - Address space pages total
6 - Largest range that can be allocated

7 - Free conventional memory
8 - Largest conventional memory block
9 - Percentage of extended memory pages locked (0-100 integer)

10- Previous page faults per second value
```

For maximum accuracy, disable preemption, especially when performing a multi-step calculation with results.

Example
```
LONG MemoryInUsePercent()
{
	// Disable
	S_PreemptInc();
	LONG v = M_GetInfo(0)*100 / M_GetInfo(1)*100;
	// Enable
	S_PreemptDec();
}
```

> Conventional memory must always be 640K

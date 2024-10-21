## Memory Chains

RAM and page file space are indexed in an array called the MAT or Memory Allocation Table.

If there is no more memory, demand-paged space is allocated. A demand paging window is used to achieve orthogonality.

> Swap is an incorrect term. OS/90 does not swap.

### M_GetInfo(BYTE type)

0 - Free extended memory pages
1 - Size of extended memory
2 - Free disk backed space in pages (DBS)
3 - Size of DBS in pages

4 - Address space pages free
5 - Address space pages total
6 - Largest range that can be allocated

7 - Free conventional memory
- Largest conventional memory block

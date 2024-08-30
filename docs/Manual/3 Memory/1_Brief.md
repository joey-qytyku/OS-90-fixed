# Memory Manager

The OS/90 memory manager supports:
- Demand paging (e.g. loading a program larger than RAM)
- Multi-granularity zone-based page frame allocations
- Heuristic virtual memory allocations
- ISA DMA buffer allocation
- Single address space
- Uncommitted memory

Heap management is intentionally left out for simplicity. It is the responsibility of drivers to manage memory efficiently.

There is one shared address space. The real mode addressable region including 65536 bytes above 1M are unpaged, unswappable, and permanently mapped. As a result, real mode memory is shared by all DOS programs. This reduces available memory but improves performance because there is no need to translate from real mode to real mode.


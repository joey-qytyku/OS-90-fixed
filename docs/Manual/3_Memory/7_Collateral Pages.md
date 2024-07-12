# Collateral Pages

Collateral pages are virtual memory pages which point to locked memory but have a handler associated with their page frame allocation entry that is called when the system needs to free it immediately upon returning. There are two levels in order of priority, L0 and L1, with L0 being the least likely to be purged.

This allows device drivers to allocate memory rather egregiously for cache and prefetching purposes. This does NOT mean that a filesystem driver for example should allocate a huge chunk for caching an entire large file.

Purging collateral pages is not "free." Each page procedure needs to be called, memory writes to the PF table and reads from the PD/PT must also occur, all multiplying with the number of collateral pages.

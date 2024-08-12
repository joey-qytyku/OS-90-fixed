## Memory Manager Examples

### "Feed Me Pages"

```
LONG ch;
PVOID buff

ch = M_Alloc(8192);
buff = M_ReserveMapping(8192);
M_Map(ch, buff, PAGE_READ_WRITE | PAGE_CACHE_ENABLE);

```

> Can we call the handle or something as a far pointer? It can scan the return pointer?

### Realistic Memory Request

Requesting physical memory is not always a good idea. OS/90 supports uncommitted memory and permits random access to it that requires no more than two pages to be allocated or one page.

Uncommitted memory is slow.

### Practical Tips

Allocating more uncomitted memory than can be allocated in physical memory is pointless and dangerous because memory is used more frequently than it is freed, and each use has a chance of allocating it as well. Uncommitted memory cannot go directly to swap and must be allocated.



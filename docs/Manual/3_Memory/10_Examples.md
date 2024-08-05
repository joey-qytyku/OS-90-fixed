# Memory Manager Examples

## "Feed Me Pages"

```
LONG ch;
PVOID buff

ch = M_Alloc(8192, 0);
buff = M_ReserveMapping(8192);
M_Map(ch, buff, PAGE_READ_WRITE | PAGE_CACHE_ENABLE);
```

> Can we call the handle or something as a far pointer? It can scan the return pointer?

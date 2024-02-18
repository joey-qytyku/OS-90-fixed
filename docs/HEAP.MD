# OS/90 Heap Manager Into

The heap manager is portable and is written in C rather than assembly like most of the OS, and can be integrated into any program. The goal is to reduce memory usage and fragmentation by reshuffling memory blocks.

The OS/90 heap manager uses MacOS-style double pointers rather than opaque handles, but improves upon that design by having lock counts.

# The API

The funciton names are similar to classic MacOS but some new ones are added.

HAlloc()
HFree()
HLock()
HUnlock()
HFarPointer()

A far pointer is possible.

# Master Allocation Table (MAT)

Allocations are aligned to a 16-byte boundary, leaving 5 unused bits in the address.

```
#define H_LOCKED    1
#define H_OFFHEAP   2
```

# Defrag Algorithm

Defragmentations do not need to happen unless memory is constrained.

# Programming Considerations

Pointers to data inside a block requires special consideration. If the block is moved after locking, the pointer could become invalid.

If a pointer, perhaps in an array of structures, must reference data located on absolutely any allocated block, it is impractical to lock every block in question. This would reduce the benefit of reshuffling.

In C, array subscripts have higher precedence than pointer dereferences. This means that `*data[foo]` would not work correctly.

The entire heap is locked with a single mutex and is thread-safe.

# Example Code

```
#include <Heap.h>

int main()
{
    char **data = HAlloc(4000);

    HLock(data);

    strcpy(*data, "Hello, world");

    HFree(data);
}

```


> Should we switch back to pascal case?

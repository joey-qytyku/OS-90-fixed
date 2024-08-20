# Kernel Heap

The memory manager functionality related to chains is wholly inadequate for common tasks done by many drivers. It is very inefficient to allocate a 4K multiple every time a small buffer is needed. A consistent way to manage memory exists in the form of a handle-based heap manager.

The heap is intended for small items that are kept in memory for a long time by drivers but are smaller than a page. It is _not_ a fast allocator by any measure, but is not that slow either.

```
PVOID M_HAlloc(LONG bytes)
VOID M_HFree(PVOID handle)
M_HLock(PVOID handle)
M_HUnlock(PVOID handle)
LONG M_HTotalAvail(VOID)
LONG M_HGetHandlesLeft(VOID)
LONG M_HGetSize(PVOID handle)
STAT M_HValidate(VOID)
```

Classic MacOS-style double pointers are used. Handles are `void*` so that they count as a generic pointer and can be casted to a double pointer type without errors.

## PVOID M_HAlloc(LONG bytes)

Returns a void pointer that as a value represents the handle. Dereferencing it further goes to the allocated region.

## When To Validate

It is recommended to validate after large buffer transfers to unsure no overruns took place. Remember that heap validation is quite slow.

## Warnings

### C Syntax

When accessing arrays, remember that the array subscript has higher precedence than the pointer dereference.

```
// Correct way
PLONG *data = M_HAlloc();
(*data)[0] = 40;
```

Lock counts are defined as no larger 254. After that, an overlocked block is reported after heap validation.

The structure member operator has the same operator precedence as the subscript.

### Never PermaLock

Never lock memory and leave it that way. This is a terrible idea because the heap manager is dependent on moving memory to avoid fragmentation.

Ensure than locking is done for the shortest length of time possible.

### Beware of Sentinel Bytes

16 bytes are allocated to detect buffer overflows. Do not place these bytes in the heap.
```
AB CD DE FE
88 AA 55 5A
A5 C1 BA 9A
8A 9B EE 9E
```

### Pointers To Block

Any pointer to data in the block is subject to invalidation if the block is unlocked.

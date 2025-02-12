================================================================================
	Malloc Notes
================================================================================

I will write my own malloc rather than use an existing one.

Conforming to the single address space of both OS/90, there is one memory arena shared by all programs that are clients to libc.

It is designed for 32-bit computers only.

This malloc will be added to the kernel.

--------------------------------------------------------------------------------
	Memory Model
--------------------------------------------------------------------------------

Memory is allocated in pages. There is no resizable heap in OS/90. There is nothing wrong with this since sbrk has been obsolete for a very long time.

The heap is controlled by a linked list of page-sized structures called the HBRT (heap block reference table) which contain a complex set of fixed-size structures called HCBs.

The first one has a special meaning. It contains a bitmap of every entry currently being used. It cannot be used to allocate anything and its space is reserved.

Each entry is 128 bytes long, which allows a single 32-bit bsf instruction to be used to find an allocation to use. It also allows for an instant comparison to be used for checking if there are any free blocks available before moving on to the next one or allocating another table.

```
struct _hcb_first {
	uint32_t        alloc_mask;
	void *          prev_fhcb;
	void *          next_fhcb;
};

struct _hcb {
	uint8_t                 sentiel[3];
	int8_t                  realloc_count;
	uint32_t                bytes;
	struct _hcb*            next;
	struct _hcb*            prev;
};

// sizeof(struct _hcb) == sizeof(struct _hcb_first)
```

Allocations that are too large to fit require the same data structure. The difference is that they are sparsely allocated.


There are two different types of non-small requests:
- LR: Larger than 4096 byte allocations go straight to the operating system and are allocated in pages.

- MR: Larger than the small request but smaller than a page.

MR is quite complicated as it requires organizing the memory into another structure.


--------------------------------------------------------------------------------
	Medium Requests
--------------------------------------------------------------------------------

Medium requests do not fit inside the small buffers but it gets a location off the heap.

The same heap structure handles these requests but they get a different arena.

> I can achieve a allocation size hierarchy by using a very similar page-based allocation model. I could have a "page pair" with a control block and allocatable memory after.


## Tracking Usage Patterns

A set of information fields is used by all requests structures to indicate:

- Was the block expanded previously?
- Was the block reduced?
-



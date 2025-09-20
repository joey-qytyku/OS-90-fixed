# M_Alloc

```c
void* M_Alloc(	uint	commit,
		uint	uncommit,
		uint	page_bits,
		uint	flags,
		const char *label
)
```

`page_bits` uses the official x86 format. OS/90 uses reserved bits for other information.

`label` is a C string with no more than 8 characters. Used for debugging or checking resource usage.

Values for flags:
|Option|Effect|
-|-
ALLOC_XD | Uncommitted pages are allocated on the bottom, useful for stacks.



# Complexity

This function currently has a worst-case complexity of O(n) where n is the number of blocks allocated on the entire system. It uses a doubly linked list. Virtual address space is also allocated similarly.

This function is NOT faster than malloc under the vast majority of circumstances. There is also no resizing supported at all, so it should not be used for anything expected to grow.

Also, the kernel uses this to allocate space for thread contexts and VM information. 4K for each thread and a total of 8K for a virtual DOS machine.


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

There are two types of requests:
- Small:	124 bytes or less	(32/frame)
- Medium:	380 bytes or less	(10/frame)
- Large:	1020 bytes or less	(4/frame)
- Extra large	Page granular

Memory is allocated in pages. There is no resizable heap in OS/90. There is nothing wrong with this since sbrk has been obsolete for a very long time.

Memory manager structures are stored in something called a "frame." Each type of request gets a different frame structure, or does not if it is page granular.

--------------------------------------------------------------------------------
	Common Heap Header
--------------------------------------------------------------------------------

In order to allow allocations to be freed irresepective of what kind of allocation it is,

We know it is an page granular allocation if the alignment of the pointer is to a page. That way we can immediately use a page free operation. OS/90 can free several pages using a single pointer.

Otherwise, a header structure is used that makes the alignment 16-byte.

--------------------------------------------------------------------------------
	Small
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
	Medium Requests
--------------------------------------------------------------------------------

Medium requests do not fit inside the small buffers but it gets a location off the heap.

The same heap structure handles these requests but they get a different arena.

> I can achieve a allocation size hierarchy by using a very similar page-based allocation model. I could have a "page pair" with a control block and allocatable memory after.


## Notes

Because I do not use a complex heap structure with regular headers, there is a slighly missed opportunity: tracking resizes.

Here, resizes have the potential to be slow if we do not keep track of the real size of the allocation before copying to a larger block.

We will need a 32-bit size specifier, perhaps after the sentinel byte. It can be validated for correctness (i.e. fits inside the frame).

I also need to have a way of knowing immediately what type of mframe ana allocation belongs to.

Only the smallest one uses a bit field to find free blocks.

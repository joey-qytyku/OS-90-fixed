#include <stdint.h>
#include <stdlib.h>

#define BYTE uint8_t
#define SHORT uint16_t
#define LONG  uint32_t

#define PBYTE BYTE*
#define PSHORT SHORT*
#define PLONG LONG*

#define STATUS LONG

// This is a macro. We want this to be inlined as much as possible.
#define ROUNDUP_BYTES2UNITS(V, A) ( (V+(A)) & (-(A)) )


typedef struct {
	// The largest block that can be allocated.
	SHORT   largest_block;

	// The number of unallocated chunks
	LONG    free_chunks;

	// The size of the heap including all chunks
	LONG    chunks_total;

	// Pointer to MPT
	P_MPE   mpt;

	// Pointer to the heap data
	PVOID   data;
}MHT;

typedef struct {
	PVOID   pointer;
	BYTE    lock;
	BYTE    free_chunks_after;
	BYTE    alloc_size;
	BYTE    pad;
}MPE,*P_MPE;

MHT g_heap;

PVOID M_HAlloc(LONG bytes)
{
	static LONG good_indices[48];
	static BYTE good_index = 0;

	LONG alloc_chunks = ROUNDUP_BYTES2UNITS(bytes, 16) + 1;
	LONG i = 0;

	P_MPE ;

	while (1) {
		if (i == 512)
			goto could_not_find;

		// Is the block unallocated but with memory?
		if (g_heap.mpt[i].pointer != NULL) {
			good_indices[good_index]
			good_index++;
		}


		i++;
	}

	could_not_find:
}

VOID M_HFree(PVOID handle)
{}

M_HLock(PVOID handle)
{}

M_HUnlock(PVOID handle)
{}

LONG M_HTotalAvail(VOID)
{}

LONG M_HGetSize(LONG)
{}

STAT M_HValidate(LONG);
{}

STAT M_HInit(MHT *heap, LONG heap_size)
{
	g_heap.data             = aligned_alloc(16, heap_size);
	g_heap.mpt              = aligned_malloc(4096, 4096);
	g_heap.chunks_total     = heap_size / 16;
	g_heap.free_chunks      = heap_size / 16;

	memset(heap->mpt, 0, 4096);

	return 0;
}

int main()
{
	M_HInit(&heap, 65536);

}

/*
	Copright (C) 2025 Joey Qytyku, All Rights Reserved
*/

// Restyle the defined in printf

// All three must be defined.
#if !defined(MALLOC_SYM) || !defined(CALLOC_SYM) || !defined(FREE_SYM)
	#define MALLOC_SYM _malloc
	#define CALLOC_SYM _calloc
	#define FREE_SYM   _free
#endif

#define SIZE_TYPE unsigned

#ifndef GET_PAGES
	#include <stdlib.h>
	#define GET_PAGES(N) memalign((N)*4096U, 4096)
#endif

#define SENT 0xAA55

typedef struct _hdr_self {
	unsigned short		wSentinel;
	unsigned short		wType;
	unsigned		dAllocMask;
	void*			pNext;
	void*			pPrev;
}hdr;

// enum {S,M,L,XL,PG};

static void * ffptrtab[4] = {NULL, NULL, NULL, NULL};

static short stride_tab[4] = {128, 256, 512, 1024};

static unsigned type_tab[5] = {[0] = 0, [2] = 1, [4] = 2, [8] = 3};

static unsigned init_mask[4] = {0xFFFFFFFF, 0xFFFF, 0xFF, 0xF};

void *MALLOC_SYM(SIZE_TYPE n)
{
	unsigned stride;
	unsigned type;

	// Points to the first entry in the frame at all times,
	// or the previous one.
	hdr *ch = NULL, ph = NULL;

	if (n <= 1024-sizeof(hdr)) {
		inx = type_tab[n>>7];
		stride = stride_tab[type];
		ch = ffprttab[type];
	}
	else {
		return GET_PAGES((n + 4095) & (~4096));
	}
	while (1) {
		if (ch == NULL) {
			ch = GET_PAGES(1);

			// Setup the first entry special fields.
			ch->dAllocMask = init_mask[inx];
			ch->pNext = NULL;
			ch->pPrev = ph;

			// Set each entry appropriately.
			for (unsigned i = 0; i < 4096; i += stride) {
				*((hdr*)((void*)ch+i)) = (hdr){SENT, type};
			}
		}

		// Bit scan goes here.

		ph = ch;
		ch = ((void *)(ch->next))+stride;
	}
}

int InitMalloc()
{
}

int main()
{
}

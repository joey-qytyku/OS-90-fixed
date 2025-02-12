#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#include <fcntl.h>

// Check the assembler output of this.

// Note: upgrade to 64-bit bitmap.
//
// Should we?
//

struct _srhdr {
	uint32_t        sentinel;
	uint32_t        alloc_mask;
	struct _srhdr * prev_srhdr;
	struct _srhdr * next_srhdr;

	// May add more. Not even close to filling.
};

// Do we even need an MCB at all? If we go with no heap validation whatsoever,
// then perhaps. I think there should be at least a sentinel word.

//
// I have to consider if this data should be process-local or not.
// In the context of the kernel, it will not make any sense.
//
static struct _mcb_first *F = NULL;

// Change to access the local structure for userspace.
static inline struct _srhdr *_get_first_srhdr(void)
{
	return F;
}

static inline void *_get_pages(size_t pages)
{
	return memalign(pages * 4096, 4096);
}

// Small requests are size-agnostic.
static void *_malloc_small(void)
{
	struct _srhdr *f = _get_first_srhdr();

	do {
		if (f == NULL) {
			void *np = _get_pages(1);
			if (np == NULL) {
				return NULL;
			}
		}
		else {
			if ()
		}

		f = f->next_srhdr;
	}
	while (f->next_srhdr != NULL);
}

void *_malloc(size_t c)
{
	if (c <= 124U) {
		return _malloc_small();
	}
}

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include <stdint.h>

typedef int32_t SIGLONG;
typedef int16_t SIGSHORT;
typedef int8_t  SIGBYTE;

typedef uint32_t LONG;
typedef uint16_t SHORT;
typedef uint8_t  BYTE;

typedef LONG  *PLONG;
typedef SHORT *PSHORT;
typedef BYTE  *PBYTE;

typedef const BYTE *PCSTR;

#define STAT LONG

#define VOID void
#define PVOID void*
#define BOOL _Bool

typedef struct _ZENT {
	PVOID   prev;
	PVOID   next;
	LONG    rel_index;
	PVOID   _side_link;
}ZENT,*PZENT;

typedef struct {
	union {
		struct {
			LONG    id; /* "MPAT", "PPAT", etc. */
			LONG    blocks_alloced;
			LONG    blocks_remaining;
			PZENT   ztab;
			BYTE    grnl_pow2;
			BYTE    unused[3];
		};
		LONG	pad[32];
	};
}ZONED,*PZONED;

VOID Z_CreateZone(
	PZONED  z,
	LONG    entries,
	BYTE    grnl,
	PZENT   ztab)
{
	printf("Allocating %i blocks for pool\n", entries);

	for (int i = 0; i < entries; i++) {
		ztab[i].prev = NULL;
		ztab[i].next = NULL;
		ztab[i].rel_index = 0xFFFFFFFF;
	}

	z->blocks_alloced = 0;
	z->grnl_pow2 = grnl;
	z->blocks_remaining = entries;
	z->ztab = ztab;
}

// This is wrong crap?
#define ROUNDUP_BYTES2UNITS(V, A) ( (V+(A)-1) & (-(A)) )

// Bytes has a limited range for current input. I need to check it.
PZENT Z_Alloc(PZONED z, LONG bytes)
{
	const LONG blocks_alloc = ROUNDUP_BYTES2UNITS(bytes, 1<<z->grnl_pow2) / (1<<z->grnl_pow2);
	LONG    did_alloc       = 0;
	PZENT   cur             = z->ztab;
	PZENT   last_ent        = NULL;
	PZENT   retval          = NULL;

	printf("Allocating     | %i\n", blocks_alloc);
	printf("From zone pool | %i\n", z->blocks_remaining);

	// Ensure that allocation can succeed
	// if ((SIGLONG)z->blocks_remaining-(SIGLONG)z->blocks_alloced < 0) {
	// 	return NULL;
	// }

	if (blocks_alloc > z->blocks_remaining) {
		return NULL;
	}

	while (1)
	{
		if (did_alloc == blocks_alloc)
			break;
		// printf(">%i\n", cur->rel_index);
		if (cur->rel_index == 0xFFFFFFFF)
		{
			// puts("Found one");
			if (did_alloc == 0) {
				printf("Found first\n");
				retval = cur;
				printf("Worked\n");
				goto skip_for_first;
			}

			cur->next = NULL;
			cur->prev = last_ent;
			last_ent->next = cur;
			// We still have to zero things. What things? Pointers?

			skip_for_first:
			cur->rel_index = did_alloc;

			last_ent = cur;
			did_alloc++;
			// printf("did_alloc: %i\n", did_alloc);
		}
		cur++;
	}
	// We have to update the counts
	z->blocks_remaining -= blocks_alloc;
	return retval;
}
// Notes:
//      cur->rel_index = did_alloc;
//
// This works because did_alloc has not been updated yet at this point.

//      goto skip_for_first_point;
//
//

STAT Z_Free(PZONED z, PZENT chain)
{
	PZENT cur = chain;

	while (1) {
		cur->rel_index = 0xFFFFFFFF;

		if (cur->next == NULL) {
			break;
		}
		cur = cur->next;
	}
	// We have to update things
	return 0;
}

VOID Z_AddToGroup(PZENT group_of, PZENT toadd)
{
	group_of->
}

STAT Z_FreeGroup(PZENT group_of)
{
	PZENT cur = group_of;
	while (cur != NULL) {
		Z_Free();
		cur = cur->next;
	}
}

VOID ReadTable(PZONED z)
{
	puts("-----------------");
	int i;
	int end;
	scanf("%i %i", &i, &end);
	end += i;

	for (i ; i < end; i++) {
		printf(
			"================| ENTRY AT %p\n"
			"Prev #   | 0%xh\n"
			"Next #   | 0%xh\n"
			"Rel  #   | 0%xh\n",
			&z->ztab[i],
			(LONG)z->ztab[i].prev,
			(LONG)z->ztab[i].next,
			(LONG)z->ztab[i].rel_index
			// ((LONG)z->ztab[i].prev - (LONG)z->ztab) / sizeof(ZENT),
			// ((LONG)z->ztab[i].next - (LONG)z->ztab) / sizeof(ZENT),
			// z->ztab[i].rel_index
		);
	}
}

ZONED zone;
ZENT  table[8192];

int main(int argc, char **argv)
{
	Z_CreateZone(&zone, 1024, 12, table);
	// PZENT a1 = ZBA_Alloc(&zone, 4096*1024);

	// ReadTable(&zone);

	return 0;
}

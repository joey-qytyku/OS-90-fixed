/*
	Copright (C) 2025 Joey Qytyku, All Rights Reserved
*/

/*******************************************************************************
*******************************************************************************/


// All three must be defined.
#if !defined(MALLOC_SYM) || !defined(CALLOC_SYM) || !defined(FREE_SYM)
	#define MALLOC_SYM _malloc
	#define CALLOC_SYM _calloc
	#define FREE_SYM   _free
#endif

#define SIZE __SIZE_TYPE__

#ifndef GET_PAGES
	#include <stdlib.h>
	#define GET_PAGES(N) (memalign((N)*4096U, 4096U))
#endif

#define SEN 0x2CDDF129

typedef struct {
	void *next;
	void *prev;
	unsigned mask;
	char arena_flag;
	short pad;
}hdr;

static hdr *arenas[5];

//
// A lookupup table implements the bit scan instruction. It is MUCH faster than
// a regular BSF instruction on 3/4/586.
//
// Larger scans are implemented by checking mutiple bytes.
//
#include "scanlut.h"

static unsigned bit_scan(unsigned char b)
{
	return b > 0 ? lut[b] : -1;
}

static void malloc_base(SIZE s, int arena)
{
	hdr *h = arenas[arena];

}

void *MALLOC_SYM(SIZE s)
{
}

void *_calloc(size_t n, size_t s)
{}

void _free(void *p)
{}

int InitMalloc()
{
}

int main()
{
	InitMalloc();
}

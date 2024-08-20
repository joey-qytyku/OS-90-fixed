#ifndef MEMOPS_H
#define MEMOPS_H

#include <stddef.h>

void M_fast_memcpy(void * __restrict to, void * __restrict from, size_t size);
#pragma aux M_fast_memcpy       parm   [EDI] [ESI] [ECX]\
				modify [EAX EDI ESI ECX];

void M_fast_memset(void *buff, int val, size_t size);
#pragma aux M_fast_memset       parm   [EDI] [EAX] [ECX] \
				modify [EDI EAX EBX ECX];

void M_fast_memzero(void *buff, size_t size);
#pragma aux M_fast_memzero      parm   [EDI] [ECX] \
				modify [EAX EBX ECX EDI];


void M_fast_memset2(void *buff, int val, size_t words);
#pragma aux M_fast_memset2      parm   [EDI] [EAX] [ECX] \
				modify [EAX EBX ECX EDI];
#endif /* MEMOPS_H */

#ifndef MEMOPS_H
#define MEMOPS_H

#include <stddef.h>

// TODO copy from other project

void M_fast_memcpy_(void * __restrict to, void * __restrict from, size_t size);
// #pragma aux M_fast_memcpy       parm   [EDI] [ESI] [ECX]\
// 				modify [EAX EDI ESI ECX];

void M_fast_memset_();
// #pragma aux M_fast_memset       parm   [EDI] [EAX] [ECX] \
// 				modify [EDI EAX EBX ECX];

void M_fast_memzero_(void *buff, size_t size);
// #pragma aux M_fast_memzero      parm   [EDI] [ECX] \
// 				modify [EAX EBX ECX EDI];

void M_fast_memset2_(void *buff, int val, size_t words);
// #pragma aux M_fast_memset2      parm   [EDI] [EAX] [ECX] \
// 				modify [EAX EBX ECX EDI];



#endif /* MEMOPS_H */

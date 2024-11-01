#ifndef MEMOPS_H
#define MEMOPS_H

#include <stddef.h>

extern int      M_fast_memcpy_,
		M_fast_memset_,
		M_fast_memzero_,
		M_fast_memset2_;

// REMEMBER THE CLOBBERS!!
// Honestly, I should probably save the registers.
// Loops will probably need the registers
// For now, no.

__attribute__((always_inline))
static inline void M_fast_memcpy(
	void * __restrict to,
	void * __restrict from,
	size_t size)
{
	__asm__ volatile(
		"call M_fast_memcpy_"
		:
		: "D" (to), "S" (from), "c" (size)
		: "edi", "esi", "ecx", "memory", "cc"
	);
}

__attribute__((always_inline))
static inline void M_fast_memzero(
	void *buff,
	size_t size)
{
	__asm__ volatile(
		"call M_fast_memzero_"
		:
		:"D"(buff),"c"(size)
		:"memory","eax","ecx","edi","cc"
	);
} // GOOD

//
// This does NOT work 100% like stdlib memset.
//
__attribute__((always_inline))
static inline void M_fast_memset(
	void *buff,
	int val,
	size_t size)
{
	__asm__ volatile(
		"call M_fast_memset_"
		:
		:"a"(val), "D"(buff),"c"(size)
		:"memory","cc"
	);
} // GOOD

__attribute__((always_inline))
static inline void M_fast_memset2(
	void *buff,
	int val,
	size_t size)
{
	__asm__ volatile(
		"call M_fast_memset2_"
		:
		:"a"(val), "D"(buff),"c"(size)
		:"memory","cc"
	);
} // GOOD

#endif /* MEMOPS_H */

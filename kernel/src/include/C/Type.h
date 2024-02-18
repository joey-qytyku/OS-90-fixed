////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                     Copyright (C) 2023, Joey Qytyku                        //
//                                                                            //
// This file is part of OS/90 and is published under the GNU General Public   //
// License version 2. A copy of this license should be included with the      //
// source code and can be found at <https://www.gnu.org/licenses/>.           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef TYPE_H
#define TYPE_H

#include <stdbool.h>

/////////////////////////////////////////////////////////////////////////////
// I n l i n e   A s s e m b l y   a n d   A s s e m b l y   L i n k a g e //
/////////////////////////////////////////////////////////////////////////////

#define ASM_LINK __attribute__(( regparm(0) ))

////////////////////////////////////////////////////
// F u n c t i o n   R e l a t e d   M a c r o s  //
////////////////////////////////////////////////////

#define kernel  __attribute__(( regparm(0), cdecl ))
#define kernel_async kernel
#define BYTESWAP(value) ((value & 0xFF) << 24) | ((value & 0xFF00) << 8) |\
((value & 0xFF0000)>>8) | ((value & 0xFF000000) >> 24)

#define UNUSED_PARM(x) (VOID)x

// The procedure returns a pointer with a predefined boundary
// This allows GCC to optimize code with more clever assumtions

// Drivers cannot possible know the alignment of certain
// things like memory blocks. bound should NEVER be a macro!!!!!
#define RETPTR_WITH_FIXED_BOUND(bound) __attribute__((alloc_align(bound)))

#define ARGPTR_RDO(...)  __attribute__( (access(read_only,  __VA_ARGS__ )) )
#define ARGPTR_WRO(...)  __attribute__( (access(write_only, __VA_ARGS__ )) )
#define ARGPTR_RDWR(...) __attribute__( (access(read_write, __VA_ARGS__ )) )

// API_DECL creates a type definition for a function
// so that it can be added to the procedure table

#define API_DECL(rtype, name, ...)\
    typedef rtype kernel (*_API_##name)(__VA_ARGS__);\
    rtype kernel name(__VA_ARGS__)

///////////////////////////////////////////
// M i s c e l a n e o u s   M a c r o s //
///////////////////////////////////////////

#define ALIGN(x) __attribute__( (aligned(x)) )

#define FENCE(xpr)\
__asm__ volatile ("":::"memory");\
xpr;\
__asm__ volatile ("":::"memory")

#define offsetof(st, m) ((U32)&(((st *)0)->m))

/////////////////////////////////////
// T y p e   D e f i n i t i o n s //
/////////////////////////////////////

typedef __UINT32_TYPE__ uint;
typedef __UINT16_TYPE__ ushort;

typedef uint size_t;
typedef int  ssize_t;

typedef __UINT64_TYPE__ U64,*PU64;

#define NULL ((PVOID)0UL)

#define likely(x)   __builtin_expect((x),1)
#define unlikely(x) __builtin_expect((x),0)

//
// Pascal string support for C using 16-bit size specifiers.
// Works using hacky inline assembly, but totally worth it.
// Pascal strings are vastly superior.
//
#define PSTRING(name, value)\
asm(#name "_len: .word " #name "_end-" #name);\
asm(#name ": .ascii " "\"" value "\"" "\n");\
asm(#name "_end:\n");\
extern char name[] asm(#name "_len");

typedef struct {
  ushort len;
  char   dat[];
}pstring_t;

/////////////////////////////////////////////////////////////////////
// A d d r e s s + O f f s e t   A d d r e s s i n g   M a c r o s //
/////////////////////////////////////////////////////////////////////

#define BYTE_PTR(var, off) *(unsigned char) ((var)+(off))
#define WORD_PTR(var, off) *(ushort) ((var)+(off))
#define DWORD_PTR(var,off) *(uint)((var)+(off))

/////////////////////////////
// G C C   B u i l t i n s //
/////////////////////////////

// Macrofy?
static inline void *_memcpy(void *d, void *s, size_t c)
{
	return __builtin_memcpy(d, s, c);
}

static inline void *_memmove(void *d, void *s, size_t c)
{
	return __builtin_memmove(d, s, c);
}

// Replace with memcmp and use pstrings
static inline int _strcmp(char *s1, char *s2)
{
	return __builtin_strcmp(s1, s2);
}

static inline void _memset(void *a, int val, size_t count)
{
    __builtin_memset(a,val,count);
}

#endif /* TYPE_H */

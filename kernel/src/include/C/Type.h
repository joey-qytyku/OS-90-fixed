/*******************************************************************************
                        Copyright (C) 2023, Joey Qytyku

This file is part of OS/90 and is published under the GNU General Public
License version 2. A copy of this license should be included with the
source code and can be found at <https://www.gnu.org/licenses/>.
*******************************************************************************/

#ifndef TYPE_H
#define TYPE_H

#include <stdbool.h>

/////////////////////////////////////////////////////////////////////////////
// I n l i n e   A s s e m b l y   a n d   A s s e m b l y   L i n k a g e //
/////////////////////////////////////////////////////////////////////////////


// We need to start using this for ASM functions
#define ASM_LINK __attribute__(( regparm(0) ))

#define NORETURN _Noreturn

////////////////////////////////////////////////////
// F u n c t i o n   R e l a t e d   M a c r o s  //
////////////////////////////////////////////////////

#define noinline __attribute__((noinline))

// TODO
#define kernel_export

#define BYTESWAP(value) ((value & 0xFF) << 24) | ((value & 0xFF00) << 8) |\
((value & 0xFF0000)>>8) | ((value & 0xFF000000) >> 24)

#define UNUSED_PARM(x) (VOID)x

// API_DECL creates a type definition for a function
// so that it can be added to the procedure table

// TODO!
#define API_DECL(rtype, name, ...)

    // typedef rtype kernel (*_API_##name)(__VA_ARGS__);\
    // rtype kernel name(__VA_ARGS__)

///////////////////////////////////////////
// M i s c e l a n e o u s   M a c r o s //
///////////////////////////////////////////

#define ALIGN(x) __attribute__( (aligned(x)) )

#define FENCE(xpr)\
        __asm__ volatile ("":::"memory");\
        {xpr};\
        __asm__ volatile ("":::"memory")

#define NULL ((PVOID)0UL)

/////////////////////////////////////
// T y p e   D e f i n i t i o n s //
/////////////////////////////////////

typedef __UINT32_TYPE__ LONG;
typedef __UINT16_TYPE__ SHORT;
typedef __UINT8_TYPE__  BYTE;

typedef __UINT32_TYPE__ *PLONG;
typedef __UINT16_TYPE__ *PSHORT;
typedef __UINT8_TYPE__  *PBYTE;

typedef void VOID;
typedef void *PVOID;

#define BOOL bool

/////////////////////////////////////////////////////////////////////
// A d d r e s s + O f f s e t   A d d r e s s i n g   M a c r o s //
/////////////////////////////////////////////////////////////////////

// THESE ARE DEAD WRONG
#define _BYTE_PTR(var, off) *(BYTE) ((PVOID)(var)+(off))
#define _WORD_PTR(var, off) *(SHORT) ((PVOID)(var)+(off))
#define _DWORD_PTR(var,off) *(LONG)((PVOID)(var)+(off))

/////////////////////////////
// G C C   B u i l t i n s //
/////////////////////////////

#define likely(x)   __builtin_expect((x),1)
#define unlikely(x) __builtin_expect((x),0)

#define memcpy(d, s, c) __builtin_memcpy(d, s, c);
#define memmove(d, s, c) __builtin_memmove(d, s, c);
#define strcmp(s1, s2) __builtin_strcmp(s1, s2);
#define memset(a, val, count) __builtin_memset(a,val,count)

#endif

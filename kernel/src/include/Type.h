/*******************************************************************************
		      Copyright (C) 2022-2024, Joey Qytyku

  This file is part of OS/90.

  OS/90 is free software. You may distribute and/or modify it under
  the terms of the GNU General Public License as published by the
  Free Software Foundation, either version two of the license or a later
  version if you choose.

  A copy of this license should be included with OS/90.
  If not, it can be found at <https://www.gnu.org/licenses/>
*******************************************************************************/

#ifndef TYPE_H
#define TYPE_H

#define EOK   0
#define EFAIL 1

/////////////////////////////////////////////////////////////////////////////
// I n l i n e   A s s e m b l y   a n d   A s s e m b l y   L i n k a g e //
/////////////////////////////////////////////////////////////////////////////

#define NORETURN _Noreturn

////////////////////////////////////////////////////
// F u n c t i o n   R e l a t e d   M a c r o s  //
////////////////////////////////////////////////////

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

#define NULL ((PVOID)0UL)

/////////////////////////////////////
// T y p e   D e f i n i t i o n s //
/////////////////////////////////////

typedef unsigned long  LONG;
typedef unsigned short SHORT;
typedef unsigned char  BYTE;

typedef LONG  *PLONG;
typedef SHORT *PSHORT;
typedef BYTE  *PBYTE;

typedef long  SIGLONG;
typedef short SIGSHORT;
typedef char  SIGBYTE;

typedef const BYTE *PCSTR;

#define VOID void
#define PVOID void*
#define BOOL _Bool

/////////////////////////////////////////////////////////////////////
// A d d r e s s + O f f s e t   A d d r e s s i n g   M a c r o s //
/////////////////////////////////////////////////////////////////////

// Fixed
#define BYTE_PTR(var, off) *(BYTE)  ((PVOID)(var) + (off))
#define WORD_PTR(var, off) *(SHORT) ((PVOID)(var) + (off))
#define DWORD_PTR(var,off) *(LONG)  ((PVOID)(var) + (off))

/////////////////////////////
// G C C   B u i l t i n s //
/////////////////////////////

#endif /* TYPE_H */

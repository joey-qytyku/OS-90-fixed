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

#define OS_OK   0
#define OS_ERR  1

/////////////////////////////////////////////////////////////////////////////
// I n l i n e   A s s e m b l y   a n d   A s s e m b l y   L i n k a g e //
/////////////////////////////////////////////////////////////////////////////

#define NORETURN _Noreturn

////////////////////////////////////////////////////
// F u n c t i o n   R e l a t e d   M a c r o s  //
////////////////////////////////////////////////////

// TODO
#define APICALL

#define BYTESWAP(value) ((value & 0xFF) << 24) | ((value & 0xFF00) << 8) |\
((value & 0xFF0000)>>8) | ((value & 0xFF000000) >> 24)

#define UNUSED_PARM(x) (VOID)x

///////////////////////////////////////////
// M i s c e l a n e o u s   M a c r o s //
///////////////////////////////////////////

#define NULL ((PVOID)0UL)

#define MK_FNPTR(RTYPE, NAME, ARGS) typedef RTYPE(*NAME) ARGS

/////////////////////////////////////
// T y p e   D e f i n i t i o n s //
/////////////////////////////////////

typedef unsigned long  LONG;
typedef unsigned short SHORT;
typedef unsigned char  BYTE;

typedef LONG   *PLONG;
typedef SHORT *PSHORT;
typedef BYTE  *PBYTE;

typedef int   SIGLONG;
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

// Create a register that has a 32-bit/16-bit/8-bit high or low version
#define _DWB_R(n)\
union {\
	LONG E##n##X;\
	union {\
		BYTE n##L;\
		struct { BYTE :8; BYTE n##H; };\
	};\
	SHORT n##X;\
}

// Create a register that can be 16-bit or 32-bit.
#define _DW_R(n)\
union {\
	LONG E##n;\
	SHORT n;\
}

/*******************************************************************************
This is the standard register dump structure. It is also known as a context
and contains every register that represents an execution state in any mode
of execution within the OS/90 protected mode environment.

Stdregs is the structure that is used by the low-level interrupt handler,
system entry, and task management.

It is important to understand that this is simply to promote the reuse of
code. Stdregs can be used in many situations that are not always related.

Stdregs can have various meanings and does not always have to be allocated
to its full extent. The V86 segment registers at the end are only generated
in cases when switching from a V86 context. alloca can be used in such
situations.

When entering V86, the V86 registers are used, but should be
accessed using the ES,DS,FS,GS,CS names since the v86 ones are a bit of an
implementation detail.
*******************************************************************************/
typedef struct {
	_DWB_R(A);
	_DWB_R(B);
	_DWB_R(C);
	_DWB_R(D);

	_DW_R(SI);
	_DW_R(DI);
	_DW_R(BP);

	LONG   pm_ES;
	LONG   pm_DS;
	LONG   pm_FS;
	LONG   pm_GS;
	_DW_R(IP);

	LONG   CS;
	_DW_R(FLAGS);
	_DW_R(SP);
	LONG   SS;

	LONG v86_ES;
	LONG v86_DS;
	LONG v86_FS;
	LONG v86_GS;
}STDREGS, *PSTDREGS;

#undef _DW_R
#undef _DWB_R

#endif /* TYPE_H */

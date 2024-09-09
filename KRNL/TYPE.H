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

#include <stdbool.h>

/////////////////////////////////////////////////////////////////////////////
// I n l i n e   A s s e m b l y   a n d   A s s e m b l y   L i n k a g e //
/////////////////////////////////////////////////////////////////////////////

#define NORETURN _Noreturn

#define API __attribute__((cdecl))

#define API_DECL(rtype, name, ...)\
	typedef rtype API (*_API_##name)(__VA_ARGS__);\
	rtype API name(__VA_ARGS__)


#define NULL ((PVOID)0UL)

// Need?
#define MK_FNPTR(RTYPE, NAME, ...) typedef RTYPE(*NAME) (__VA_ARGS__)

typedef unsigned long  LONG;
typedef unsigned short SHORT;
typedef unsigned char  BYTE;

typedef LONG   *PLONG;
typedef SHORT *PSHORT;
typedef BYTE  *PBYTE;

typedef int   SIGLONG;
typedef short SIGSHORT;
typedef char  SIGBYTE;

typedef int   *PSIGLONG;
typedef short *PSIGSHORT;
typedef char  *PSIGBYTE;

#define VOID void
#define PVOID void*
#define BOOL bool

#define unlikely(x) __builtin_expect((x),0)
#define likely(x)   __builtin_expect((x),1)

// Remove?
#define DWORD_PTR(var,off) *(LONG)  ((PVOID)(var) + (off))
#define WORD_PTR(var, off) *(SHORT) ((PVOID)(var) + (off))
#define BYTE_PTR(var, off) *(BYTE)  ((PVOID)(var) + (off))

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

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

#define inline_memcpy(d, s, c) __builtin_memcpy((d), (s), (c))
#define inline_memset(d, v, c) __builtin_memset((d), (v), (c))

#define BIT(x) (1<<(x))

#define API __attribute__((cdecl))

#define API_DECL(rtype, name, ...)\
	typedef rtype API (*_API_##name)(__VA_ARGS__);\
	rtype API name(__VA_ARGS__)


//
// In OS/90, NULL is actually 0xFFFFFFFF because 0
// is a valid address.
//
// This MUST be taken into consideration. E.g.
// structures with NULL pointers cannot just be
// zeroed.
//
#define OSNULL ((PVOID)0xFFFFFFFF)

#ifdef NULL
#warning "OS/90 does not use NULL. See related documentation."
#endif

// Need?
// #define MK_FNPTR(RTYPE, NAME, ...) typedef RTYPE(*NAME) (__VA_ARGS__)

typedef unsigned long  LONG;
typedef unsigned short SHORT;
typedef unsigned char  BYTE;

typedef LONG  *PLONG;
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
struct {\
	union {\
		LONG E##n##X;\
		union {\
			SHORT n##X;\
			struct { BYTE n##L; BYTE n##H; };\
		}__attribute__((packed));\
	};\
};

// Create a register that can be 16-bit or 32-bit.
#define _DW_R(n)\
struct {\
	union {\
		LONG E##n;\
		SHORT n;\
	};\
};

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

*******************************************************************************/

typedef struct __attribute__((packed)) {
	_DWB_R(A);
	_DWB_R(B);
	_DWB_R(C);
	_DWB_R(D);

	_DW_R(SI);
	_DW_R(DI);
	_DW_R(BP);

	SHORT   pm_ES;
	SHORT   :8;
	SHORT   pm_DS;
	SHORT   :8;
	SHORT   pm_FS;
	SHORT   :8;
	SHORT   pm_GS;
	SHORT   :8;

	_DW_R(IP);
	SHORT   CS;
	SHORT   :8;

	#define I86_C   BIT(0)
	#define I86_PF  BIT(2)
	#define I86_AF  BIT(4)
	#define I86_ZF  BIT(6)
	#define I86_SF  BIT(7)
	#define I86_TF  BIT(8)
	#define I86_IF  BIT(9)
	#define I86_DF  BIT(10)
	#define I86_OF  BIT(11)
	#define I86_IOPL (BIT(12) | BIT(13))
	#define I86_NT  (BIT(14))
	#define I86_VM  BIT(17)
	#define I86_AC  BIT(18)
	_DW_R(FLAGS);

	_DW_R(SP);
	LONG   SS;

	LONG v86_ES;
	LONG v86_DS;
	LONG v86_FS;
	LONG v86_GS;
	#undef _DW_R
	#undef _DWB_R
}STDREGS, *PSTDREGS;

// This is to stop code that may do something that is not implemented yet
// for testing purposes, to see if an interface is required.
#define NOT_IMPLEMENTED() {__asm__ volatile("xchg %bx,%bx");}

#endif /* TYPE_H */

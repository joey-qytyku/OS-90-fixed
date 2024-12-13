/////////////////////////////////////////////////////////////////////////////
//                     Copyright (C) 2022-2024, Joey Qytyku                //
//                                                                         //
// This file is part of OS/90.                                             //
//                                                                         //
// OS/90 is free software. You may distribute and/or modify it under       //
// the terms of the GNU General Public License as published by the         //
// Free Software Foundation, either version two of the license or a later  //
// version if you chose.                                                   //
//                                                                         //
// A copy of this license should be included with OS/90.                   //
// If not, it can be found at <https://www.gnu.org/licenses/>              //
/////////////////////////////////////////////////////////////////////////////

#ifndef TYPE_H
#define TYPE_H

#define OS_OK   0
#define OS_ERR  1

#include <stdbool.h>

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

#define inline_memcpy(d, s, c) __builtin_memcpy((d), (s), (c))
#define inline_memset(d, v, c) __builtin_memset((d), (v), (c))

#define BIT(x) (1<<(x))

#define API __attribute__((stdcall))

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

#define unlikely(x) __builtin_expect((x),0)
#define likely(x)   __builtin_expect((x),1)

typedef struct __attribute__((packed)) {
	// Create a register that has a 32-bit/16-bit/8-bit high or low version
	#define _DWB_R(n)\
	union {\
		LONG    E##n##X; \
			BYTE n##L;\
		struct {BYTE __##n; BYTE n##H; }__attribute__((packed));\
		SHORT   n##X; \
	}

	// Create a register that can be 16-bit or 32-bit.
	#define _DW_R(n)\
	union {\
		LONG E##n;\
		SHORT n;\
	};

	#define I86_C   0x0001
	#define I86_PF  0x0004
	#define I86_AF  0x0010
	#define I86_ZF  0x0040
	#define I86_SF  0x0080
	#define I86_TF  0x0100
	#define I86_IF  0x0200
	#define I86_DF  0x0400
	#define I86_OF  0x0800
	#define I86_IOPL 0x3000
	#define I86_NT  0x4000
	#define I86_VM  0x00020000
	#define I86_AC  0x00040000

	LONG    EAX;
	LONG    EBX;
	LONG    ECX;
	LONG    EDX;

	LONG    ESI;
	LONG    EDI;
	LONG    EBP;

	LONG    pm_ES;
	LONG    pm_DS;
	LONG    pm_FS;
	LONG    pm_GS;

	_DW_R(IP);
	LONG    CS;

	_DW_R(FLAGS);

	_DW_R(SP);

	LONG    SS;

	LONG v86_ES;
	LONG v86_DS;
	LONG v86_FS;
	LONG v86_GS;
	#undef _DW_R
	#undef _DWB_R
}REGS, *PREGS;

// This is to stop code that may do something that is not implemented yet
// for testing purposes, to see if an interface is required.
#define NOT_IMPLEMENTED() {__asm__ volatile("xchg %bx,%bx":::"memory");}

#endif /* TYPE_H */

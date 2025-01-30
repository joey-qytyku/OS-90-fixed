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

// I will remove this soon
#define inline_memcpy(d, s, c) __builtin_memcpy((d), (s), (c))
#define inline_memset(d, v, c) __builtin_memset((d), (v), (c))

#define API __attribute__((stdcall))

#define API_DECL(rtype, name, ...)\
	typedef rtype API (*_API_##name)(__VA_ARGS__);\
	rtype API name(__VA_ARGS__)

#define fromto(name, from, to) for (LONG name = (from); (name) < (to); (name)++)

#define BIT(n) (1<<(n))

//
// In OS/90, NULL is actually 0xFFFFFFFF because 0
// is a valid address.
//
// This MUST be taken into consideration. E.g.
// structures with NULL pointers cannot just be
// zeroed.
//
#define OSNULL ((PVOID)0xFFFFFFFFU)

#ifdef NULL
#warning "OS/90 does not use NULL. See related documentation."
#endif

#define unlikely(x) __builtin_expect((x),0)
#define likely(x)   __builtin_expect((x),1)

typedef struct __attribute__((packed)) {
	#define I86_C       0x0001U
	#define I86_PF      0x0004U
	#define I86_AF      0x0010U
	#define I86_ZF      0x0040U
	#define I86_SF      0x0080U
	#define I86_TF      0x0100U
	#define I86_IF      0x0200U
	#define I86_DF      0x0400U
	#define I86_OF      0x0800U
	#define I86_IOPL    0x3000U
	#define I86_NT      0x4000U
	#define I86_VM      0x00020000U
	#define I86_AC      0x00040000U

	union { LONG EAX; SHORT AX; struct { BYTE AL; BYTE AH; }; };
	union { LONG EBX; SHORT BX; struct { BYTE BL; BYTE BH; }; };
	union { LONG ECX; SHORT CX; struct { BYTE CL; BYTE CH; }; };
	union { LONG EDX; SHORT DX; struct { BYTE DL; BYTE DH; }; };

	union { LONG ESI; SHORT SI; };
	union { LONG EDI; SHORT DI; };
	union { LONG EBP; SHORT BP; };

	LONG    pm_ES;
	LONG    pm_DS;
	LONG    pm_FS;
	LONG    pm_GS;

	union { LONG EIP; SHORT IP; };
	LONG    CS;

	union { LONG EFLAGS;    SHORT FLAGS; };
	union { LONG ESP;       SHORT SP; };

	LONG SS;

	LONG v86_ES;
	LONG v86_DS;
	LONG v86_FS;
	LONG v86_GS;
}REGS, *PREGS;

// This is to stop code that may do something that is not implemented yet
// for testing purposes, to see if an interface is required.
#define NOT_IMPLEMENTED() {__asm__ volatile("xchg %bx,%bx":::"memory");}

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

#ifndef IO_H
#define IO_H

static inline void rep_insb(void* mem, LONG count, SHORT port)
{
    __asm__ volatile(
        "cld\n"
        "rep insb"
        : "=D"(mem), "=c"(count)
        : "D"(mem), "c"(count), "d"(port)
        : "memory"
    );
}

static inline void rep_outsb(void *mem, LONG count, SHORT port)
{
    __asm__ volatile(
        "cld\n"
        "rep outsb"
        :
        : "S"(mem), "c"(count), "d"(port)
        : "esi", "ecx", "edx", "memory"
    );
}

static inline void rep_insw(void *mem, LONG count, SHORT port)
{
    __asm__ volatile(
        "cld\n"
        "rep insw"
        :
        : "D"(mem), "c"(count), "d"(port)
        : "edi", "ecx", "edx", "memory"
    );
}

static inline void rep_outsw(void *mem, LONG count, SHORT port)
{
    __asm__ volatile(
        "cld\n"
        "rep outsw;"
        :
        : "S"(mem), "c"(count), "d"(port)
        : "esi", "ecx", "edx", "memory"
    );
}

#define _MAKE_PORT_IN(_ASM_TYPE_PREFIX, _TYPE)\
static inline _TYPE in##_ASM_TYPE_PREFIX(SHORT port)\
{\
    _TYPE ret;\
    __asm__ volatile ("in" #_ASM_TYPE_PREFIX " %1, %0" :"=a"(ret) :"Nd"(port):"memory");\
    return ret;\
}

#define _MAKE_PORT_OUT(_ASM_TYPE_PREFIX, _TYPE)\
static inline void out##_ASM_TYPE_PREFIX (SHORT port, _TYPE val)\
{\
    __asm__ volatile ("out" #_ASM_TYPE_PREFIX " %0, %1": :"a"(val), "Nd"(port):"memory");\
}

_MAKE_PORT_OUT(b, BYTE);
_MAKE_PORT_OUT(w, SHORT);
_MAKE_PORT_OUT(l, LONG);

_MAKE_PORT_IN(b, BYTE);
_MAKE_PORT_IN(w, SHORT);
_MAKE_PORT_IN(l, LONG);

static inline void delay_outb(SHORT port, BYTE val)
{
    outb(port, val);
    outb(0x80, 0); // Output to unused port for delay
}

static inline BYTE delay_inb(SHORT port)
{
    outb(0x80, 0);
    return inb(port);
}

#undef _MAKE_PORT_OUT
#undef _MAKE_PORT_IN

#endif /* IO_H */

#endif /* TYPE_H */

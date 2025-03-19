/////////////////////////////////////////////////////////////////////////////
//	 Copyright (C) 2022-2024, Joey Qytyku	//
//	 //
// This file is part of OS/90.	 //
//	 //
// OS/90 is free software. You may distribute and/or modify it under	   //
// the terms of the GNU General Public License as published by the	 //
// Free Software Foundation, either version two of the license or a later  //
// version if you chose.	   //
//	 //
// A copy of this license should be included with OS/90.	   //
// If not, it can be found at <https://www.gnu.org/licenses/>	  //
/////////////////////////////////////////////////////////////////////////////

#ifndef TYPE_H
#define TYPE_H

#define API __attribute__((cdecl, regparm(0)))

#define API_DECL(rtype, name, ...)\
	typedef rtype API (*_API_##name)(__VA_ARGS__);\
	rtype API name(__VA_ARGS__)

#define BIT(n) (1U<<(n))

#define OSNULL ((PVOID)0xFFFFFFFFU)

#if defined(__OS90_WANT_NULL__)
#define NULL OSNULL
#endif

#define unlikely(x) __builtin_expect((x),0)
#define likely(x)   __builtin_expect((x),1)

// Rules about REGS:
// - A pointer stored inside here must not point to a valid object in the given
//   context if an existing pointer exists.
//
//

typedef struct __attribute__((packed)) {
	#define I86_C		0x0001U
	#define I86_PF		0x0004U
	#define I86_AF		0x0010U
	#define I86_ZF		0x0040U
	#define I86_SF		0x0080U
	#define I86_TF		0x0100U
	#define I86_IF		0x0200U
	#define I86_DF		0x0400U
	#define I86_OF		0x0800U
	#define I86_IOPL	0x3000U
	#define I86_NT		0x4000U
	#define I86_VM		0x00020000U
	#define I86_AC		0x00040000U

	union {
		unsigned EAX;
		unsigned short AX;
		struct { unsigned char AL; unsigned char AH; };
	};
	union {
		unsigned EBX;
		unsigned short BX;
		struct { unsigned char BL; unsigned char BH; };
	};
	union {
		unsigned ECX; unsigned short CX;
		struct { unsigned char CL; unsigned char CH; };
	};
	union {
		unsigned EDX; unsigned short DX;
		struct { unsigned char DL; unsigned char DH; };
	};

	union { unsigned ESI; unsigned short SI; };
	union { unsigned EDI; unsigned short DI; };
	union { unsigned EBP; unsigned short BP; };

	unsigned pm_ES;
	unsigned pm_DS;
	unsigned pm_FS;
	unsigned pm_GS;

	union { unsigned EIP; unsigned short IP; };
	unsigned CS;

	union { unsigned EFLAGS;	unsigned short FLAGS; };
	union { unsigned ESP;		unsigned short SP; };

	unsigned SS;

	unsigned v86_ES;
	unsigned v86_DS;
	unsigned v86_FS;
	unsigned v86_GS;
}REGS;


static inline void rep_insb(void* mem, unsigned count, unsigned short port)
{
	__asm__ volatile(
	"cld\n"
	"rep insb"
	: "=D"(mem), "=c"(count)
	: "D"(mem), "c"(count), "d"(port)
	: "memory"
	);
}

static inline void rep_outsb(void *mem, unsigned count, unsigned short port)
{
	__asm__ volatile(
	"cld\n"
	"rep outsb"
	:
	: "S"(mem), "c"(count), "d"(port)
	: "esi", "ecx", "edx", "memory"
	);
}

static inline void rep_insw(void *mem, unsigned count, unsigned short port)
{
	__asm__ volatile(
	"cld\n"
	"rep insw"
	:
	: "D"(mem), "c"(count), "d"(port)
	: "edi", "ecx", "edx", "memory"
	);
}

static inline void rep_outsw(void *mem, unsigned count, unsigned short port)
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
static inline _TYPE in##_ASM_TYPE_PREFIX(unsigned short port)\
{\
	_TYPE ret;\
	__asm__ volatile ("in" #_ASM_TYPE_PREFIX " %1, %0" :"=a"(ret) :"Nd"(port):"memory");\
	return ret;\
}

#define _MAKE_PORT_OUT(_ASM_TYPE_PREFIX, _TYPE)\
static inline void out##_ASM_TYPE_PREFIX (unsigned short port, _TYPE val)\
{\
	__asm__ volatile ("out" #_ASM_TYPE_PREFIX " %0, %1": :"a"(val), "Nd"(port):"memory");\
}

_MAKE_PORT_OUT(b, unsigned char);
_MAKE_PORT_OUT(w, unsigned short);
_MAKE_PORT_OUT(l, unsigned int);

_MAKE_PORT_IN(b, unsigned char);
_MAKE_PORT_IN(w, unsigned short);
_MAKE_PORT_IN(l, unsigned int);

static inline void delay_outb(unsigned short port, unsigned char val)
{
	outb(port, val);
	outb(0x80, 0); // Output to unused port for delay
}

static inline unsigned char delay_inb(unsigned short port)
{
	outb(0x80, 0);
	return inb(port);
}

#undef _MAKE_PORT_OUT
#undef _MAKE_PORT_IN

#define rep_movsl(d,s,c)\
__asm__ volatile ( \
	"movl %0,%%edi\n\t"\
	"movl %1,%%esi\n\t"\
	"movl %2,%%ecx\n\t"\
	"rep movsl"::"rmi"((d)), "rmi"((s)), "rmi"((c))\
	:"memory", "edi", "esi", "ecx"\
)\

#define rep_movsw(d,s,c) \
__asm__ volatile ( \
	"movl %0,%%edi\n\t"\
	"movl %1,%%esi\n\t"\
	"movl %2,%%ecx\n\t"\
	"rep movsw"::"rmi"((d)), "rmi"((s)), "rmi"((c))\
	:"memory", "edi", "esi", "ecx"\
)\

#define rep_movsb(d,s,c) \
__asm__ volatile ( \
	"movl %0,%%edi\n\t"\
	"movl %1,%%esi\n\t"\
	"movl %2,%%ecx\n\t"\
	"rep movsb"::"rmi"((d)), "rmi"((s)), "rmi"((c))\
	:"memory", "edi", "esi", "ecx"\
)\

#define rep_movsd rep_movsl

#define rep_stosd(d,v,c) \
__asm__ volatile ( \
	"movl %0,%%edi\n\t" \
	"movl %1,%%eax\n\t" \
	"movl %2,%%ecx\n\t" \
	"rep stosl" \
	::"rmi"((d)),"rmi"((v)),"rmi"((c)) \
	:"memory","eax","ebx","ecx","edi" \
)

// Automatic version that checks for count size if it is constant.
// Tested and working.
#define rep_movs(d,s,c) \
{ \
	if (__builtin_constant_p(c)) {\
		if ((c) % 4U == 0)	rep_movsl((d),(s),(c)/4U); \
		else if ((c) % 2 == 0)	rep_movsw((d),(s),(c)/2U); \
		else			rep_movsb((d),(s),(c)); \
	}else rep_movsb((d),(s),(c));\
}

// __attribute__((force_inline))
// static inline peek32(unsigned *p)
// {
// 	__asm__ volatile (:::"memory");
// }

#endif /* TYPE_H */

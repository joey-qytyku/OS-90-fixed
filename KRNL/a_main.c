/////////////////////////////////////////////////////////////////////////////
//                     Copyright (C) 2022-2025, Joey Qytyku                //
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

#include "l_segmnt.h"
#include "centcom.h"

#include "task.h"
#include "sv86.h"

#define fromto(name, from, to) for (unsigned name=(from); (name) < (to); (name)++)

enum {
	GDT_NULL        = 0x00,
	GDT_CSEG        = 0x08,
	GDT_DSEG        = 0x10,
	GDT_TSS         = 0x18,
	GDT_LDT         = 0x20,
	GDT_RM_CS       = 0x28,
	GDT_RM_DS       = 0x30,
	GDT_WIN16       = 0x38
};

struct tss {
	short   _[25*2+1];
	short   bitmap_base;
	char	bitmap[8192];
}__attribute__((packed));

typedef struct __attribute__((packed)) {
	unsigned short	limit;
	unsigned	base;
}DESC_TAB_REG;

typedef struct __attribute__((packed)) {
	unsigned short	off1;
	unsigned short	cseg;
	unsigned char	_res;
	unsigned char	access;
	unsigned short	off2;
}IDT_ENTRY;

extern char EXC_0;
extern char ISR_1, ISR_REST, IRQ0;
extern char END_DATA;
extern char BSS_SIZE;

#define ALIGN_CACHE __attribute__(( __aligned__(CACHE_LINE_BOUNDARY) ))

ALIGN_CACHE SEGMENT_DESCRIPTOR	gdt[8];
ALIGN_CACHE SEGMENT_DESCRIPTOR	ldt[128];
ALIGN_CACHE struct tss			TSS;
ALIGN_CACHE IDT_ENTRY			idt[256];
ALIGN_CACHE DESC_TAB_REG		gdtr={63,(unsigned)&gdt};
ALIGN_CACHE DESC_TAB_REG		idtr={2047,(unsigned)&idt};

__attribute__((cold))
static void SetIsr(void *addr, unsigned i)
{
	idt[i].off1 = (unsigned)((unsigned)addr) & 0xFFFF;
	idt[i].off2 = (unsigned)((unsigned)addr) >> 16;
	idt[i].cseg = 0x8;
	idt[i]._res = 0;
	idt[i].access = 0b10001110;
}

__attribute__((cold))
static void SetupStructures(void)
{
	static const unsigned char
		access_cseg     = 0x9A,
		access_dseg     = 0x92,
		access_tss      = 0x89,
		access_ldt      = 0x82;

	static const unsigned
		limit_tss       = sizeof(ldt)-1,
		base_tss        = (unsigned)&TSS,
		base_ldt        = (unsigned)ldt,
		limit_ldt       = sizeof(ldt)-1;




	#define SC L_SegmentCreate
	/* Selector     Base address    Limit        Access rights   ExtA
	---------------------------------------------------------------------*/
SC(     GDT_CSEG,    0,              0xFFFFFF,       access_cseg,    0xC0  );
SC(     GDT_DSEG,    0,              0xFFFFFF,       access_dseg,    0xC0  );
SC(     GDT_TSS,     base_tss,       104+8192,       access_tss,     0x00  );
SC(     GDT_LDT,     limit_ldt,      limit_tss,      access_ldt,     0     );
SC(     GDT_WIN16,   0x400,          255,            access_ldt,     0     );
SC(     GDT_RM_CS,   0xFFFF0,        0xFFFF,         access_cseg,    0     );
SC(     GDT_RM_DS,   0xFFFF0,        0xFFFF,         access_dseg,    0     );
	#undef SC

	// Technically only 20 are supported ATM.
	fromto(i, 0, 32) SetIsr(&EXC_0 + i*16, i);

	fromto(i, 0, 15) SetIsr(&ISR_1+(i)*16, i+0xA1);

	SetIsr(&IRQ0, 0xA0);

	TSS.bitmap_base = 104;

	// Note: LTR marks busy. This does not matter because we never enter
	// task state segments anyway.
	__asm__ volatile(
	"       lidt idtr\n"
	"       lgdt gdtr\n"
	"       mov $0x20,%%ax\n"
	"       lldt %%ax\n"
	"       mov $0x18,%%ax\n"
	"       ltr %%ax\n"
	"       mov $0x10,%%ax\n"
	"       mov %%ax,%%ds\n"
	"       mov %%ax,%%es\n"
	"       mov %%ax,%%ss\n"

	"       xor %%ax,%%ax\n"
	"       mov %%ax,%%fs\n"
	"       mov %%ax,%%gs\n"
	"       jmpl $0x8,$cont__;"
	"       cont__:"
		:::"memory","eax"
	);
}

// 1 MS frequency
__attribute__((cold))
static void ConfigurePIT(void)
{
	const unsigned char count[2] = {0xB0, 0x4};

	outb(0x43, 0x36);
	outb(0x40, count[0]);
	outb(0x40, count[1]);
}

__attribute__((noreturn))
void KernelMain(void)
{
	// Zero BSS first, important.
	// May want to add sanity check int the mm code.
	memset((void*)&END_DATA, 0, (unsigned)&BSS_SIZE);
	asm("xchg %bx,%bx");

	SetupStructures	();
	asm("xchg %bx,%bx");
	ConfigurePIT	();
	asm("xchg %bx,%bx");
	RemapPIC	();
	asm("xchg %bx,%bx");
	InitV86		();
	asm("xchg %bx,%bx");

	__asm__ volatile(
	"       mov %%cr0,%%eax;        "
	"       orl $(1<<16),%%eax;     "
	"       mov %%eax,%%cr0         "
		:::"memory", "eax"
	);

	__asm__ volatile(
	"       mov %%cr3,%%eax;        "
	"       mov %%eax,%%cr3;        "
		:::"memory","eax"
	);

	asm("xchg %bx,%bx");
	asm("sti");

	__asm__ volatile("jmp .":::"memory");
}

__attribute__(( __noreturn__, __naked__, __section__(".init") ))
void EntryPoint(void)
{
	asm("xchg %bx,%bx");
	__asm__ ("movl $(0x100000+65520), %esp");
	__asm__ ("jmp KernelMain");
}

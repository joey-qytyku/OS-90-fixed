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

#include "l_segmnt.h"
#include "centcom.h"

#include "task.h"
#include "sv86.h"
#include "z_io.h"

#include "printf.h"

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

struct tss
{
	SHORT   _[25*2+1];
	SHORT   bitmap_base;
	BYTE    bitmap[8192];
}__attribute__((packed));

typedef struct __attribute__((packed)) {
	SHORT   limit;
	LONG    base;
}DESC_TAB_REG;

typedef struct __attribute__((packed)) {
	SHORT   off1;
	SHORT   cseg;
	BYTE    _res;
	BYTE    access;
	SHORT   off2;
}IDT_ENTRY;

extern char EXC_0;
extern char ISR_1, ISR_REST, IRQ0;
extern char END_DATA;
extern char BSS_SIZE;

SEGMENT_DESCRIPTOR gdt[8];
SEGMENT_DESCRIPTOR ldt[128];

// V86 is special and requires an IOPB ALONE to dictate
// if it should be direct.
// We have an allow bitmap for that and setting the IOPB base
// to something else denies all.
struct tss TSS;

__attribute__(( aligned(64) )) IDT_ENTRY idt[256];

DESC_TAB_REG gdtr = {63,   (LONG)&gdt};
DESC_TAB_REG idtr = {2047, (LONG)&idt};

// BYTE rmca[];

static VOID SetIsr(PVOID addr, BYTE i)
{
	idt[i].off1 = (LONG)((LONG)addr) & 0xFFFF;
	idt[i].off2 = (LONG)((LONG)addr) >> 16;
	idt[i].cseg = 0x8;
	idt[i]._res = 0;
	idt[i].access = 0b10001110;
}

static VOID Gdt_Ldt_Idt_Tss_Tr(VOID)
{
	static const BYTE access_cseg = 0x9A;
	static const BYTE access_dseg = 0x92;
	static const BYTE access_tss  = 0x89;
	static const BYTE access_ldt  = 0x82;

	static const LONG limit_tss = sizeof(ldt)-1;
	static const LONG base_tss  = (LONG)&TSS;

	static const LONG base_ldt  = (LONG)ldt;
	static const LONG limit_ldt = sizeof(ldt)-1;

	#define SC L_SegmentCreate
	/* Selector     Base address    Limit           Access rights   ExtA
	---------------------------------------------------------------------*/
	SC(GDT_CSEG,    0,              0xFFFFFF,       access_cseg,    0xC0);
	SC(GDT_DSEG,    0,              0xFFFFFF,       access_dseg,    0xC0);
	SC(GDT_TSS,     base_tss,       104+8192,       access_tss,     0x00);
	SC(GDT_LDT,     limit_ldt,      limit_tss,      access_ldt,     0);
	SC(GDT_WIN16,   0x400,          255,            access_ldt,     0);
	SC(GDT_RM_CS,   0xFFFF0,        0xFFFF,         access_cseg,    0);
	SC(GDT_RM_DS,   0xFFFF0,        0xFFFF,         access_dseg,    0);
	#undef SC

	// Technically only 20 are supported ATM.
	for (LONG i = 0; i < 32; i++)
		SetIsr(&EXC_0 + i*16, i);

	for (LONG i = 0; i < 15; i++)
		SetIsr(&ISR_1+(i)*16, i+0xA1);

	SetIsr(&IRQ0, 0xA0);

	TSS.bitmap_base = 104;

	// Note: LTR marks busy. This does not matter because we never enter
	// task state segments anyway.
	__asm__ volatile(
		"lidt idtr\n"
		"lgdt gdtr\n"
		"mov $0x20,%%ax\n"
		"lldt %%ax\n"
		"mov $0x18,%%ax\n"
		"ltr %%ax\n"
		"mov $0x10,%%ax\n"
		"mov %%ax,%%ds\n"
		"mov %%ax,%%es\n"
		"mov %%ax,%%ss\n"

		"xor %%ax,%%ax\n"
		"mov %%ax,%%fs\n"
		"mov %%ax,%%gs\n"
		"jmpl $0x8,$cont__;"
		"cont__:"
		:::"memory","eax"
	);
}

static VOID ConfigurePIT(VOID)
{
	const BYTE count[2] = {0xB0, 0x4};

	outb(0x43, 0x36);
	outb(0x40, count[0]);
	outb(0x40, count[1]);
}

static VOID ConfigutePIC(VOID)
{

}

static void pc(char c)
{
	static REGS r = {0};
	r.AH = 2;
	r.DL = c;
	// r.SS = 0x8000;
	// r.ESP = 4096;
	INTxH(0x21, &r);
}

const char mystr[] = "Hello from INT 21H!\n\r$";

static void TestINT21h()
{
	REGS r = {0};
}

VOID KernelMain(VOID)
{
	// Zero BSS first, important

	inline_memset((PVOID)&END_DATA, 0, (LONG)&BSS_SIZE);

	Gdt_Ldt_Idt_Tss_Tr();

	ConfigurePIT();

	RemapPIC();

	InitV86();

	__asm__ volatile(
		"mov %%cr0,%%eax;"
		"orl $(1<<16),%%eax;"
		"mov %%eax,%%cr0"
		:::"memory", "eax"
	);

	__asm__ volatile(
		"mov %%cr3,%%eax; mov %%eax,%%cr3; sti"
		:::"memory","eax");

	__asm__ volatile("mov $0xDEAD,%%eax; jmp .":::"memory");
}

__attribute__(( __noreturn__, __naked__, __section__(".init") ))
VOID EntryPoint(VOID)
{
	__asm__ ("movl $(0x100000+65520), %esp");
	__asm__ ("jmp KernelMain");
}

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

#include "l_switch.h"
#include "l_segmnt.h"
#include "centcom.h"

#include "task.h"
#include "sv86.h"
#include "z_io.h"

#include "printf.h"

struct tss
{
	LONG back_link;
	LONG esp0;
	LONG ss0;
	LONG esp1;
	LONG ss1;
	LONG esp2;
	LONG ss2;
	LONG cr3;
	LONG eip;
	LONG eflags;
	LONG eax, ecx, edx, ebx;
	LONG esp, ebp, esi, edi;
	LONG es;
	LONG cs;
	LONG ss;
	LONG ds;
	LONG fs;
	LONG gs;
	LONG ldt;
	SHORT :16;
	SHORT bitmap_base;
	BYTE bitmap[8192];
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

extern BYTE _binary_L_SWITCH_BIN_start;
extern BYTE _binary_L_SWITCH_BIN_size;

enum {
	GDT_NULL        = 0x00,
	GDT_CSEG        = 0x08,
	GDT_DSEG        = 0x10,
	GDT_TSS         = 0x18,
	GDT_LDT         = 0x20,
	GDT_RMCS_CODE   = 0x28,
	GDT_RMCS_DATA   = 0x30,
	GDT_WIN16       = 0x38
};

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

	{
		L_SegmentCreate(
			GDT_CSEG,
			0,
			0xFFFFFF,
			access_cseg,
			0xC0
		);
		L_SegmentCreate(
			GDT_DSEG,
			0,
			0xFFFFFF,
			access_dseg,
			0xC0
		);
		L_SegmentCreate(
			GDT_TSS,
			(LONG)&TSS,
			104+8192,
			access_tss,
			0x00
		);
		L_SegmentCreate(
			GDT_LDT,
			(LONG)&ldt,
			sizeof(ldt)-1,
			access_ldt,
			0
		);

		L_SegmentCreate(
			GDT_WIN16,
			0x400,
			255,
			access_ldt,
			0
		);

		L_SegmentCreate(
			GDT_RMCS_CODE,
			0xFFFF0,
			0xFFFF,
			access_cseg,
			0
		);

		L_SegmentCreate(
			GDT_RMCS_DATA,
			0xFFFF0,
			0xFFFF,
			access_dseg,
			0
		);
	}

	{
		// Technically only 20 are supported ATM.
		for (LONG i = 0; i < 32; i++) {
			SetIsr(&EXC_0 + i*16, i);
		}
	}

	{
		// There are three entry points
		// ISR_15, ISR_7, and ISR_REST
		// IRQ#0 is handled with a different IDT entry.

		// Set IDT entries for 1-6
		for (LONG i = 0; i < 15; i++)
			SetIsr(&ISR_1+(i)*16, i+0xA1);

		SetIsr(&IRQ0, 0xA0);

	}

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
	// outb(0xE9, c);
}

const char mystr[] = "Hello from INT 21H!\n\r$";

static void TestINT21h()
{
	// REGS r;
	// r.SS = 0x8000;
	// r.ESP = 4096;
	// r.EAX = 0x200;
	// r.EDX = 'A';
	// INTxH(0x21, &r);

	// REGS r;
	// r.SS = 0x8000;
	// r.ESP = 4096;
	// r.EAX = 0x0E00|'A';
	// r.EBX = 0;
	// INTxH(0x10, &r);

	inline_memcpy((PVOID)0x80000+4096, mystr, sizeof(mystr));
	REGS r = {0};
	r.EAX = 0x900;
	r.EDX = 4096;
	r.v86_DS = 0x8000;
	r.SS = 0x8000;
	r.ESP = 4096;
	r.EFLAGS = I86_IF;
	INTxH(0x21, &r);

}

VOID KernelMain(VOID)
{
	// Zero BSS first, important

	inline_memset((PVOID)&END_DATA, 0, (LONG)&BSS_SIZE);

	Gdt_Ldt_Idt_Tss_Tr();

	// Copy RMCS data
	inline_memcpy(  (PVOID)0x103000,
			l_switch_bin,
			l_switch_bin_len
	);

	ConfigurePIT();

	RemapPIC(); // Rewite in C

	InitV86();

	__asm__ volatile(
		"mov %%cr0,%%eax;"
		"orl $(1<<16),%%eax;"
		"mov %%eax,%%cr0"
		:::"memory", "eax"
	);

	PLONG ptr = (PLONG)0x100000 + 0x1000;
	ptr[256+0] &= ~(1<<1);
	ptr[256+1] &= ~(1<<1);
	ptr[256+2] &= ~(1<<1);

	__asm__ volatile(
		"mov %%cr3,%%eax; mov %%eax,%%cr3; sti"
		:::"memory","eax");

	FuncPrintf(putE9, "Hello, world! %x\n\r", 0xDEADBEEF);
	TestINT21h();

	FuncPrintf(putE9, "Did test\n");
	__asm__ volatile("mov $0xDEAD,%%eax; jmp .":::"memory");
}

__attribute__(( __noreturn__, __naked__, __section__(".init") ))
VOID EntryPoint(VOID)
{
	__asm__ volatile ("movl $(0x100000+65520), %esp");
	__asm__ volatile ("jmp KernelMain":::"memory");
}

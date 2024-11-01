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

#include "M_MEMOPS.H"
#include "L_SEGMNT.H"
#include "Z_IO.H"
#include "TASK.H"

#include "SV86.H"

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
	LONG trace;
	LONG bitmap_base;
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
extern char ISR_15, ISR_7, ISR_REST, IRQ0;
extern char END_DATA;
extern char BSS_SIZE;

// NULL
// CSEG
// DSEG
// TSS
// LDT
//
SEGMENT_DESCRIPTOR gdt[8];
SEGMENT_DESCRIPTOR ldt[128];

// V86 is special and requires an IOPB ALONE to dictate
// if it should be direct.
// We have an allow bitmap for that and setting the IOPB base
// to something else denies all.
struct tss TSS;

ALIGNED(64) IDT_ENTRY idt[2048];

// Base is wrong?
DESC_TAB_REG gdtr = {63,   (LONG)&gdt};
DESC_TAB_REG idtr = {2047, (LONG)&idt};

ALIGNED(4096) TASK init_thread;
ALIGNED(4096) TASK other_thread;

// BYTE rmca[];

static VOID SetIsr(PBYTE addr, BYTE i)
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

	L_SegmentCreate(
		0x8,
		0,
		0xFFFFFF,
		access_cseg,
		0xC0
	);
	L_SegmentCreate(
		0x10,
		0,
		0xFFFFFF,
		access_dseg,
		0xC0
	);
	L_SegmentCreate(
		0x18,
		(LONG)&TSS,
		sizeof(TSS)-1,
		access_tss,
		0x40
	);
	L_SegmentCreate(
		0x20,
		(LONG)&ldt,
		sizeof(ldt)-1,
		access_ldt,
		0
	);

	// Technically only 20 are supported ATM.
	for (LONG i = 0; i < 32; i++) {
		SetIsr(&EXC_0 + i*16, i);
	}

	// There are three entry points
	// ISR_15, ISR_7, and ISR_REST
	// IRQ#0 is handled with a different IDT entry.

	// Set IDT entries for 1-6
	for (LONG i = 0xA1; i <= 0xA6; i++)
		SetIsr(&ISR_REST, i);

	for (LONG i = 0xA8; i <= 0xAF; i++)
		SetIsr(&ISR_REST, i);

	SetIsr(&IRQ0, 0xA0);
	SetIsr(&ISR_7, 0xA7);
	SetIsr(&ISR_15, 0xAF);

	TSS.bitmap_base = __builtin_offsetof(struct tss, bitmap);

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
		:::"memory","ax"
	);
}

static VOID ConfigurePIT(VOID)
{
    const BYTE count[2] = {0xB0, 0x4};

    outb(0x43, 0x36);
    outb(0x40, count[0]);
    outb(0x40, count[1]);
}

static void pc(char c)
{
	// outb(0xE9, c);
	STDREGS r = {
		.AH = 0xE,
		.AL = 0x41,
		.EBX = 0,
		.EIP = IVT[0x10].ip,
		.CS  = IVT[0x10].cs,
		.SS = 0x9000,
		.ESP = 2048
	};
	LONG v = V86xH(&r);
}

// Why do I not hit more than one breakpoint?

/*
https://pdos.csail.mit.edu/6.828/2005/readings/i386/s09_06.htm

If there is no privilige escalation, the stack is not saved.
This is important. I am not supposed to copy in the stack location.

That means if I am in ring-0, I should NOT
*/

/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

VOID NORETURN KernelMain(VOID)
{
	// Zero BSS first, important

	inline_memset(&END_DATA, 0, &BSS_SIZE);

	Gdt_Ldt_Idt_Tss_Tr();

	ConfigurePIT();

	RemapPIC();

	// This is NOT WORKING
	// Is stdregs correct?
	FuncPrintf(&pc, "Hello, world\n\r");

	__asm__ volatile("jmp .":::"memory");
}

__attribute__(( noreturn, naked, section(".init") ))
VOID EntryPoint(VOID)
{
	__asm__ volatile ("movl $(init_thread+4096), %esp");
	__asm__ volatile ("jmp KernelMain":::"memory");
}

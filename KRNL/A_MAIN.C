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

// SOMETHING IS TERRIBLY WRONG WITH BSS

// NULL
// CSEG
// DSEG
// TSS
// LDT
//
//
SEGMENT_DESCRIPTOR gdt[8];
SEGMENT_DESCRIPTOR ldt[128];
BYTE tss[104];

ALIGNED(64)
IDT_ENTRY idt[2048];

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

static VOID DoTheThings(VOID)
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
		(LONG)&tss,
		103,
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

	// Note: LTR marks busy. Does it matter though?
	ASM(
		"lidt idtr;"
		"lgdt gdtr;"
		"mov $0x20,%ax;"
		"lldt %ax;"
		"mov $0x18,%ax;"
		"ltr %ax;"
		"mov $0x10,%ax;"
		"mov %ax,%ds;"
		"mov %ax,%es;"
		"mov %ax,%ss;"

		"xor %ax,%ax;"
		"mov %ax,%fs;"
		"mov %ax,%gs;"
		"jmpl $0x8,$cont__;"
		"cont__:"
	:::"memory","ax"); // Do not forget LTR
}

static VOID ConfigurePIT()
{
    const BYTE count[2] = {0xB0, 0x4};

    outb(0x43, 0x36);
    outb(0x40, count[0]);
    outb(0x40, count[1]);
}

static void pc(char c)
{
	outb(0xE9, c);
}

// The stack pointer seems wrong?
static VOID _Noreturn OtherThread(VOID)
{
	// __asm__("xchgw %bx,%bx");
	// __asm__("cli");
	// FuncPrintf(pc, "[%x]\n", init_thread.regs.ESP);
	// FuncPrintf(pc, "[%x]\n", init_thread.regs.EIP);
	// FuncPrintf(pc, "[%x]\n", init_thread.regs.ESP);

	while (1) {
		outb(0xE9, 'B');
		// __asm__(
		// 	"movl $0x8000000A,%eax;"
		// 	"movl $0x88000000,%ebx;"
		// 	"movl $0x88800000,%ecx;"
		// 	"movl $0x88880000,%edx;"
		// 	"movl $0x80000000,%esi;"
		// 	"movl $0x80000000,%edi;"
		// 	// "xchgw %bx,%bx"
		// );
	}
}

// Why do I not hit more than one breakpoint?

/*
https://pdos.csail.mit.edu/6.828/2005/readings/i386/s09_06.htm

If there is no privilige escalation, the stack is not saved.
This is important. I am not supposed to copy in the stack location.

That means if I am in ring-0, I should NOT
*/

/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

VOID _Noreturn KernelMain(VOID)
{
	// Zero BSS first, important

	__builtin_memset(&END_DATA, 0, &BSS_SIZE);

	DoTheThings();

	ConfigurePIT();

	RemapPIC();

	init_thread._prev = &other_thread;
	init_thread._next = &other_thread;
	init_thread.regs.CS = 0x8;

	other_thread._next = &init_thread;
	other_thread._prev = &init_thread;

	other_thread.regs.EAX=0x8086AA55;
	other_thread.regs.CS = 0x8;
	other_thread.regs.EIP = (LONG)&OtherThread;
	other_thread.regs.pm_ES  = 0x10;
	other_thread.regs.SS  = 0x10;
	other_thread.regs.ESP = ((PVOID)&other_thread) + 4096-4;
	other_thread.regs.pm_DS  = 0x10;
	other_thread.regs.EFLAGS = 1<<9;

	ASM("sti":::"memory");

	while (1) {
		outb(0xE9, 'A');
		outb(0xE9, '\n');
		__asm__(
			"movl $0x8000000A,%eax;"
			"movl $0x88000000,%ebx;"
			"movl $0x88800000,%ecx;"
			"movl $0x88880000,%edx;"
			"movl $0x80000000,%esi;"
			"movl $0x80000000,%edi;"
			// "xchgw %bx,%bx"
		:::"memory");
	}
}

NORETURN
NAKED
SECTION(.init)
VOID EntryPoint(VOID)
{
	__asm__ volatile ("movl $(init_thread+4096), %esp");
	__asm__ volatile ("jmp KernelMain":::"memory");
}

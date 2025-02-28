%if 0
			Copyright (C) 2025, Joey Qytyku

This file is part of OS/90.

OS/90 is free software. You may distribute and/or modify it under
the terms of the GNU General Public License as published by the
Free Software Foundation, either version two of the license or a later
version if you chose.

A copy of this license should be included with OS/90.
If not, it can be found at <https://www.gnu.org/licenses/>

********************************************************************************
	About this module
	-----------------

This malloc is a 100% external fragmentation-free implementation designed for
speed and low memory use, but mostly with a bias for speed.

The intent is to use this allocator for object-oriented stuff and for buffering
rather than long-term storage.

Memory is either allocated directly with the page allocator if it is 4K or
larger, or by using pool-style allocation that is commonly used among kernels
but generalized for all object types.

If memory is in the heap (if it can be called that) it is stored inside a
"frame" which is always a page in size.

There are the following size classes for frames:
-----------------------------------------------
 Class		Maximum bytes	# in one frame
-----------------------------------------------
 SMALL		112		32
 MEDIUM		240		16
 LARGE		496		8
 XL		1008		4
-----------------------------------------------

The size class is stored in the header so that free can delete the allocation
quickly and use the correct procedure.

The allocator is really just a dispatcher to four allocators for the different
granularities.

Overall, this is an very simple (but clever) malloc implementation.

Advice for OS/90 software is to use malloc for parsing file data, storing
temporary strings and arrays, and allocating space for anything that is not
good to put on the tiny kernel mode stack.

*******************************************************************************
%endif

%assign __COUNT 0

%macro debug 1-*
	%rep %0
		%rotate -1
		%ifstr %1
			%assign __COUNT __COUNT+1
			[section .rodata]
				__string %+ __COUNT: DB %1,0
			__?SECT?__
			push __string %+ __COUNT
		%else
			push %1
		%endif
	%endrep
	call	_printf
	add esp,4*%0
%endmacro

global _main

extern	_memalign
extern	_printf, _puts

h_wSentinel	equ 0
h_wType		equ 2
h_dAllocMask	equ 4
h_next		equ 8
h_prev		equ 12

section .data

	align	32
pFirstSmall	DD 0
pFirstMedium	DD 0
pFirstLarge	DD 0
pFirstXLarge	DD 0

section .text

	align	32
_malloc:
	push	ebx
	push	esi
	push	edi

	debug	"Allocating %u bytes, %u", dword [esp+4 +12]

	ret

InitMalloc:
	push	4096
	push	4096
	call	_memalign
	mov	[pFirstSmall],eax
	call	_memalign
	mov	[pFirstMedium],eax
	call	_memalign
	mov	[pFirstLarge],eax
	add	esp,8
	ret

_main:
	call	InitMalloc
	push	405
	call	_malloc
	add	esp,4
	ret

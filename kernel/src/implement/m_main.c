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

#include <OSK/SD/basicatomic.h>
#include <OSK/SD/sv86.h>

#include <OSK/MC/pio.h>

#include <OSK/DB/debug.h>
//INCLUDE="%@P%\..\include";"%@P%\..\mfc\include";%INCLUDE%;"%@P%\..\..\..\OS-90-fixed\kernel\src\include"

static SHORT GetCmosBytePair(SHORT base)
{
        BYTE b1, b2;

        PREEMPT_INC();

        BYTE old = inb(0x70);

        outb(0x70, base+1);
        b1 = inb(0x71);

        outb(0x70, base);
        b2 = inb(0x71);

        outb(0x70, old);

        PREEMPT_DEC();
        return (b1 << 8) | b2;
}

LONG g_extended_pages;

LONG GetExtendedMemPages()
{
        SHORT size_simple = GetCmosBytePair(0x17) >> 2;

        if (size_simple == 0xFFFF) {
                // In this case, the system has more than 64MB of memory.
                // Only a BIOS function can figure this out.
                static STDREGS regs;
        }

        return g_extended_pages;
}

extern int END_BSS;

VOID M_Init(VOID)
{
        // Remember to include memory hole in PBT

        g_extended_pages = GetExtendedMemPages();

        PLONG page_dir;

        __asm__ volatile ("movl %%cr3,%0":"=r"(page_dir)::"memory");
        page_dir = (PLONG)( ((LONG)page_dir) & 0xFFFFF000 );

        //
        // Find where the kernel was loaded and its size in pages
        //
        PVOID kernel_phys_base = (*(PLONG)(page_dir[512] & 0xFFFFF000)) & 0xFFFFF000;

        debug_log("Kernel load address: %p\n", kernel_phys_base);
}

/*
     This file is part of OS/90.

    OS/90 is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.

    OS/90 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with OS/90. If not, see <ttps://www.gnu.org/licenses/>.
*/

#ifndef _8259_H
#define _8259_H

#include <Platform/IA32.h>
#include <Platform/IO.h>

#define ICW1 1<<4
#define LEVEL_TRIGGER 1<<3
#define ICW1_ICW4 1

#define ICW4_8086 1
#define ICW4_SLAVE 1<<3

/*
 * The last exception that I need to use is #XF because it
 * is supported by the Pentium III from 1999, only x64 CPUs
 * support VMX which is the next exception after that
 * This number must not conflict with vector 0x31 because that is DPMI
 */
#define IRQ_BASE 0xA0
#define DPMI_VECTOR 0x31
#define NON_SYSTEM_VECTORS DPMI_VECTOR

#if DPMI_VECTOR <= IRQ_BASE+16 && DPMI_VECTOR >= IRQ_BASE
#error IRQ BASE OVERLAPS WITH DPMI VECTOR
#endif

//
// Memory clobber causes the statement to not be moved elsewhere to
// the compiler's convenience, useful for ensuring that memory access
// happens in the exact order specified rather than where GCC wants.
// "memory" can also be used for instructions that are not supposed to move
//
static inline VOID IntsOn (VOID) { __asm__ volatile ("sti":::"memory"); }
static inline VOID IntsOff(VOID) { __asm__ volatile ("cli":::"memory"); }

static inline DWORD GetEflags(VOID)
{
    DWORD eflags;
    __asm__ volatile (
        "pushfl\n\t"
        "popl %0\n\t"
        :"=r"(eflags)
        :
        :
    );
    return eflags;
}

// The in-service register is a bit mask with one turned on
static inline WORD InGetInService16(void)
{
    WORD in_service;

    in_service  = delay_inb(0x20);
    in_service |= delay_inb(0xA0) << 8;
    return in_service;
}

#endif /* _8259_H */

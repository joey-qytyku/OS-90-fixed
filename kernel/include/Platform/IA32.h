/*
     This file is part of OS/90.

    OS/90 is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.

    OS/90 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with OS/90. If not, see <https://www.gnu.org/licenses/>.
*//*

2022-07-08 - Refactoring, changed inlines to #defines, fixed gdt struct

*/

#ifndef IA32_H
#define IA32_H

#include <Type.h>

typedef DWORD PAGE;

#define IDT_INT386  0xE /* IF=0 on entry */
#define IDT_TRAP386 0xF /* IF unchanged on entry */

// The data/stack segment enables the BIG bit
// so that Plug-and-play BIOS recognizes it as
// a 32-bit stack
#define TYPE_TSS  0x9
#define TYPE_DATA 0x12

#define TYPE_LDT 0x2

/* Readable for PnP */
#define TYPE_CODE 0x1B
#define ACCESS_RIGHTS(present, ring, type) (present<<7 | ring<<6 | type)

#define LDT_SIZE 8192

///////////////////////////
//   Segments and IDT    //
///////////////////////////

#define MkTrapGate(vector, dpl, address)\
    {_SetIntVector(vector, 0x80 | dpl<<4 | IDT_INT386, address);}

#define MkIntrGate(vector, address)\
    {_SetIntVector(vector, 0x80 | IDT_INT386, address);}

#define PnSetBiosDsegBase(base)\
    {IaAppendAddressToDescriptor(&aqwGlobalDescriptorTable + GDT_PNP_BIOS_DS*8,\
    base);}

#define PnSetOsDsegBase(base)\
    {IaAppendAddressToDescriptor(&aqwGlobalDescriptorTable + GDT_PNP_OS_DS*8\
    (DWORD)base);}

#define PnSetBiosCsegBase(base)\
    {IaAppendAddressToDescriptor(&aqwGlobalDescriptorTable + GDT_PNPCS*8,\
    (DWORD)base);}

#define KERNEL_RESERVED_MEM_ADDR _KernelReserved4K

enum {
        GDT_NULL        =     0,
        GDT_KCODE       =     1,
        GDT_KDATA       =     2,
        GDT_UCODE       =     3,
        GDT_UDATA       =     4,
        GDT_TSSD        =     5,
        GDT_LDT         =     6,
        GDT_PNPCS       =     7,
        GDT_PNP_OS_DS   =     8,
        GDT_PNP_BIOS_DS =     9,
        GDT_SYSCALL     =     10,
        GDT_ENTRIES     =     11
};

// Previously, I used a interrupt frame structure. Instead of using that bloat,
// I just have an array of DWORDs and index them with this. Less code and does
// exactly the same thing. These are array indices. They are NOT offsets.
// Not all values are valid, depending on the context.
//
// When exiting ring 3, we need to save the segment registers. If it was V86
// the data segment registers are allready pushed to the stack. If not, they
// have to be manually saved.
//
// The solution here is slightly unusual, but most of the time, we will know
// exactly segmetn register ones should be accessed, and we have to check if
// it was a real mode or protected mode client.
//
// For example, a V86 handler can be written knowing that it will use real mode
// segments. In cases where there is ambiguity, it will be necessary to check
// which mode it exited.
//
enum {
    RD_PM_ES = 0,
    RD_PM_DS = 1,
    RD_PM_FS = 2,
    RD_PM_GS = 3,
    RD_EAX   = 4,
    RD_EBX   = 5,
    RD_ECX   = 6,
    RD_EDX   = 7,
    RD_ESI   = 8,
    RD_EDI   = 9,
    RD_EBP   = 10,
    _RD_ESP  = 11,
    RD_EIP   = 12,
    RD_CS    = 13,

    RD_EFLAGS = 14,

    // In case of inter-segment switch, there are valid indices
    RD_ESP = 15,
    RD_SS  = 16,

    // In case of inter-segment switch and VM=1, these are pushed to stack
    // by the CPU
    //
    // In case of inter-segment switch and VM=0, the protected mode segment
    // selectors must be saved. The will be stored in a different location.
    //
    RD_V86_ES = 17,
    RD_V86_DS = 18,
    RD_V86_FS = 19,
    RD_V86_GS = 20,
    RD_NUM_DWORDS
};

/////////////////////////////////
//           Externs           //
/////////////////////////////////

extern PVOID _KernelReserved4K;

extern VOID InitIA32(VOID);
extern QWORD aqwGlobalDescriptorTable;

extern VOID ASM_LINK IaAppendAddressToDescriptor(
    PVOID gdt_entry,
    DWORD address
);

static inline BOOL x86BitTestD(DWORD value, BYTE bit_inx)
{
	BOOL ret;
	__asm__ volatile (
		"bt %1, %0"
		:"=ccc"(ret)
		:"ri"(bit_inx)
        :"cc"
		);
    return ret;
}

extern VOID IaUseDirectRing3IO(VOID);
extern VOID IaUseVirtualRing3IO(VOID);

extern VOID ASM_LINK _SetIntVector(DWORD vector, DWORD _attr, PVOID address);

#endif /* IA32_H */


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

extern VOID IaUseDirectRing3IO(VOID);
extern VOID IaUseVirtualRing3IO(VOID);

extern VOID ASM_LINK _SetIntVector(DWORD vector, DWORD _attr, PVOID address);

extern DWORD ASM_LINK GetDescriptorBaseAddress(WORD selector);

#endif /* IA32_H */


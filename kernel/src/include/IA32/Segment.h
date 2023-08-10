#ifndef IA32_SEGMENT_H
#define IA32_SEGMENT_H

#include <Type.h>

// The data/stack segment enables the BIG bit
// so that Plug-and-play BIOS recognizes it as
// a 32-bit stack
#define TYPE_TSS  0x9
#define TYPE_DATA 0x12

#define TYPE_LDT 0x2

/* Readable for PnP */
#define TYPE_CODE 0x1B
#define ACCESS_RIGHTS(present, ring, type) (present<<7 | ring<<6 | type)

#define LDT_ENTRIES 64

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

// These must be imported as arrays. If I did pointers, the compiler would
// think that the first GDT/LDT entry is the base address! The compiler
// will always generate arrays as a series of bytes with a single data;
// no pointers.

extern U64 aqwGlobalDescriptorTable[GDT_ENTRIES];
extern U64 aqwLocalDescriptorTable[LDT_ENTRIES];

VOID IaAppendAddressToDescriptor(PVOID, U32);
VOID IaAppendLimitToDescriptor(PVOID, U32);
U32  IaGetBaseAddress(PVOID);

// The selector is a valid offset to the table if the botom bits are masked out.
static inline U32 GetLdescBaseAddress(U16 selector)
{
    return IaGetBaseAddress(aqwLocalDescriptorTable + (selector & 0xFFF8));
}

#endif /* IA32_SEGMENT_H */

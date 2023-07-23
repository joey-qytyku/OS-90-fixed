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

#define LDT_ENTRIES 128

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

extern QWORD *aqwGlobalDescriptorTable;
extern QWORD *aqwLocalDescriptorTable;

VOID IaAppendAddressToDescriptor(PVOID, DWORD);
DWORD IaGetBaseAddress(PVOID);

//
// Note that the limit supplied must be larger than 1M in bytes.
//
VOID IaAppendLimitToDescriptor(PVOID, DWORD);

#endif /* IA32_SEGMENT_H */

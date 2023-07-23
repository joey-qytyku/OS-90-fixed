#ifndef BIT_ARRAY_H
#define BIT_ARRAY

#include <Type.h>

VOID   KERNEL   KeDisableBitArrayEntry(PDWORD,DWORD);
BOOL   KERNEL   KeGetBitArrayEntry(PDWORD,DWORD);
STATUS KERNEL   KeAllocateBits(PDWORD,DWORD,DWORD,PDWORD);
STATUS KERNEL   AllocateOneBit(PDWORD,DWORD,PDWORD);

// These are NOT kernel exports.
VOID KeEnableBitArrayEntry(PDWORD,DWORD);
VOID KeEnableBitArrayRange(PDWORD,DWORD,DWORD);

#endif
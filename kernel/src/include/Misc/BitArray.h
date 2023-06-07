#ifndef BIT_ARRAY_H
#define BIT_ARRAY

#include <Type.h>

extern VOID KeEnableBitArrayEntry(PDWORD,DWORD);
extern VOID KERNEL KeDisableBitArrayEntry(PDWORD,DWORD);
extern BOOL KERNEL KeGetBitArrayEntry(PDWORD,DWORD);
extern STATUS KERNEL KeAllocateBits(PDWORD,DWORD,DWORD,PDWORD);

extern VOID KeEnableBitArrayRange(PDWORD,DWORD,DWORD);

#endif
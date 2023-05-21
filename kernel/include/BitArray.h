#ifndef BIT_ARRAY_H
#define BIT_ARRAY

#include <Type.h>

extern VOID KeEnableBitArrayEntry(PDWORD,DWORD);
extern VOID APICALL_REGPARM(2) KeDisableBitArrayEntry(PDWORD,DWORD);
extern BOOL APICALL_REGPARM(2) KeGetBitArrayEntry(PDWORD,DWORD);
extern VOID KeEnableBitArrayRange(PDWORD,DWORD,DWORD);
extern STATUS KeAllocateBits(PDWORD,DWORD,DWORD,PDWORD);

#endif
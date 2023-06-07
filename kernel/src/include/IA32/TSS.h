#ifndef IA32_TSS_H
#define IA32_TSS_H

#include <Type.h>

extern VOID IaUseDirectRing3IO(VOID);
extern VOID IaUseVirtualRing3IO(VOID);

extern __attribute__((regparm(1))) VOID SetESP0(DWORD);
extern __attribute__((regparm(1))) PVOID GetESP0(VOID);

#endif /* IA32_TSS_H */

#ifndef IA32_TSS_H
#define IA32_TSS_H

#include <Type.h>

// All IO is allowed except ports marked for high-level emulation
// IOPB offset is set to the IOPB
extern VOID IaUseDirectRing3IO(VOID);

// All IO is trapped and passed to drivers
// The IOPB offset is set outside the TSS limit to make it "deny all"
extern VOID IaUseVirtualRing3IO(VOID);

extern __attribute__((regparm(1))) VOID SetESP0(U32);
extern __attribute__((regparm(1))) PVOID GetESP0(VOID);

extern PU32 abIoPermissionBitmap;

#define GetAddrOfIOPB() abIoPermissionBitmap

#endif /* IA32_TSS_H */

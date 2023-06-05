#ifndef SCHEDULER_CORE_H
#define SCHEDULER_CORE_H

#include "Interrupt.h"
#include "IoDecode.h"
#include "Process.h"
#include "Sync.h"
#include "V86M.h"
#include "DPMI.h"
#include "SysEntry.h"
#include <Platform/IO.h>

static inline VOID ConfigurePIT(VOID)
{
    delay_outb(0x43, 0x36);
    delay_outb(0x40, 0x80);
    delay_outb(0x40, 0x4);
}


#define ScContextWasV86() _bWasV86
#define ScGetExceptionIndex() _dwExceptIndex
#define ScGetExceptErrorCode() _dwErrorCode

extern DWORD _dwErrorCode;
extern DWORD _dwExceptIndex;

#endif /* SCHEDULER_CORE_H */

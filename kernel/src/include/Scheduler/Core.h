#ifndef SCHEDULER_CORE_H
#define SCHEDULER_CORE_H

#include "Interrupt.h"
#include "IoDecode.h"
#include "Process.h"
#include "Sync.h"
#include "V86.h"
#include "DPMI.h"
#include <Platform/IO.h>

static inline VOID Configure_PIT(VOID)
{
    delay_outb(0x43, 0x36);
    delay_outb(0x40, 0x80);
    delay_outb(0x40, 0x4);
}

#endif /* SCHEDULER_CORE_H */

#ifndef SCHEDULER_CORE_H
#define SCHEDULER_CORE_H

#include <Platform/IO.h>

static inline VOID ConfigurePIT(VOID)
{
    delay_outb(0x43, 0x36);
    delay_outb(0x40, 0x80);
    delay_outb(0x40, 0x4);
}

#endif /* SCHEDULER_CORE_H */

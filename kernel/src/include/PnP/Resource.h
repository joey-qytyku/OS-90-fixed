///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                     Copyright (C) 2023, Joey Qytyku                       //
//                                                                           //
// This file is part of OS/90 and is published under the GNU General Public  //
// License version 2. A copy of this license should be included with the     //
// source code and can be found at <https://www.gnu.org/licenses/>.          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef PNP_RESOURCE_H
#define PNP_RESOURCE_H

#include <Type.h>
#include "Drivers.h"

typedef enum {
    UNDEFINED = 0,
    BUS_FREE  = 1,  // Available interrupt
    BUS_INUSE = 2,  // 32-bit interrupt controlled by a BUS
    RECL_16   = 3   // Legacy DOS driver interrupt, can be reclaimed
}INTERRUPT_LEVEL;

#define MAX_IO_RSC 64

#define MEM 0
#define AVAIL 0

// Resource flags byte:
//|AA|DD|C|U|S|P|
// P=PORT
// S=STD
// U=INUSE
// C=MEM_CACHEABLE
// DD=
// Uses bit fields instead of byte

#define PORT 1
#define STD 2
#define INUSE 4
#define MEM_CACHABLE 8

#define IOP_DECODE_10 0
#define IOP_DECODE_16 1
#define MEM_DECODE_24 2
#define MEM_DECODE_32 3

#define ACCESS_8  0
#define ACCESS_16 1
#define ACCESS_32 2

typedef VOID (*FP_IRQ_HANDLER)(VOID);

typedef struct __attribute__((packed))
{
    U32          start;
    U32          size:24;
    U32          alignment:24;
    PVOID          owner;
    BYTE
        is_port:1,
        is_std:1,
        inuse:1,
        mem_cachable:1,
        io_decode:2,
        io_access:2;
}IO_RESOURCE,
*PIO_RESOURCE;

typedef struct __attribute__((packed))
{
    // Each level is two bits, which means that all interrupt levels fit in
    // a single 32-bit U32

    U32           lvl_bmp;
    FP_IRQ_HANDLER  handlers[16];
    PDRIVER_HEADER  owners[16];
}INTERRUPTS,
*PINTERRUPTS;

extern STATUS KERNEL PnAddIOMemRsc(PIO_RESOURCE);

extern VOID             KERNEL  InSurrenderInterrupt();
extern VOID             KERNEL  InRegainInterrupt();
extern INTERRUPT_LEVEL  KERNEL  InGetInterruptLevel(VINT);
extern FP_IRQ_HANDLER   KERNEL  InGetInterruptHandler(VINT);
extern STATUS           KERNEL  InAcquireLegacyIRQ(VINT, FP_IRQ_HANDLER);

#endif /* PNP_RESOURCE_H */
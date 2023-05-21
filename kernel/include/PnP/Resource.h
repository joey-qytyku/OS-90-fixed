#ifndef PNP_RESOURCE_H
#define PNP_RESOURCE_H

#include <Type.h>

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

typedef struct __attribute__((packed))
{
    DWORD          start;
    DWORD          size:24;
    DWORD          alignment:24;
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
    // a single 32-bit DWORD

    DWORD           lvl_bmp;
    FP_IRQ_HANDLR   handlers[16];
    PDRIVER_HEADER  owners[16];
}INTERRUPTS,
*PINTERRUPTS;

typedef VOID (*FP_IRQ_HANDLER)(VOID);

extern STATUS PnAddIOMemRsc(PIO_RESOURCE);

extern VOID InSurrenderInterrupt();
extern VOID InRegainInterrupt();
extern INTERRUPT_LEVEL InGetInterruptLevel(VINT);
extern FP_IRQ_HANDLR InGetInterruptHandler(VINT);
extern STATUS InAcquireLegacyIRQ(VINT, FP_IRQ_HANDLR);

#endif /* PNP_RESOURCE_H */
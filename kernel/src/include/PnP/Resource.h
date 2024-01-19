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

typedef enum {
    UNDEFINED = 0,
    BUS_FREE  = 1,  // Available interrupt
    BUS_INUSE = 2,  // 32-bit interrupt controlled by a BUS
    RECL_16   = 3   // Legacy DOS driver interrupt, can be reclaimed
}INTERRUPT_CLASS;

#define MAX_IO_RSC 64

#define MEM 0
#define AVAIL 0

#define PORT            0b00000001 /* Is a port */
#define INUSE           0b00000010 /* In use */
#define MEM_CACHABLE    0b00000100 /* Memory is cachable */
#define ACCESS_8        0b00000000
#define ACCESS_16       0b00001000
#define ACCESS_32       0b00010000

// Looking at the ISA PnP spec, it seems that the memory regions are naturally aligned.
// VGA has an address space of 128K (not 256K of mem) and it is naturally aligned
// at the address 0xA0000.
// Hardware uses a bit mask to determine if it should react. This defines the size.
// Both PCI and ISA PnP do this.

//
// Memory address aliasing is a major problem with ISA.
// Computers with more than 16MB of RAM will have a smart enough super IO
// chip to avoid asserting the address signals to the ISA cards. Internal
// ISA devices (in the ISA bridge) will have extended decode.
//

// #define MEM_DECODE_24 2
// #define MEM_DECODE_32 3


typedef VOID (*FP_IRQ_HANDLER)(VOID);

// Make into one struct?
tstruct
{
    U32     start;
    U32     size:24;
    PVOID   owner;
    U16     flags;
}IO_RESOURCE,
*PIO_RESOURCE;

typedef struct __attribute__((packed))
{
    // Each level is two bits, which means that all interrupt levels fit in
    // a single 32-bit U32

    U32             class_bmp;
    FP_IRQ_HANDLER  handlers[16];
    PVOID           owners[16];
}INTERRUPTS,
*PINTERRUPTS;

extern STATUS kernel PnAddIOMemRsc(PIO_RESOURCE);

extern VOID             kernel  Surrender_Interrupt();
extern VOID             kernel  Regain_IRQ();
extern INTERRUPT_CLASS  kernel  Get_IRQ_Class(U32);
extern FP_IRQ_HANDLER   kernel  Get_Interrupt_Handler(U32);
extern STATUS           kernel  Acquire_Legacy_IRQ(U32, FP_IRQ_HANDLER);

#endif /* PNP_RESOURCE_H */
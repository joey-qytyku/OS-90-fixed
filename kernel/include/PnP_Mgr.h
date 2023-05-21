/*
     This file is part of OS/90.

    OS/90 is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.

    OS/90 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with OS/90. If not, see <ttps://www.gnu.org/licenses/>.
*/

#ifndef PNP_MGR_H
#define PNP_MGR_H

#include <DriverLib/Drivers.h>
#include <Platform/IA32.h>
#include <Type.h>

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

//
// Non-PnP drivers must be loaded first
//
#define DRV_IMPLEMENT_BUS        1
#define DRV_IMPLEMENT_DEV_PNP    2
#define DRV_IMPLEMENT_DEV_LEGACY 3

//#define PNP_FPU_PRESENT  0x01
//#define PNP_FPU_INTERNAL 0x02

typedef VOID (*FP_EVENT_HANDLER) (PVOID);

// The level-3 IRQ handler function
//
// The trap frame is always passed. First argm determines
// if the trap frame pointer contains a V86 iret frame.
//
// There are few reasons for an IRQ handler to want to access the context.
// Its just there if absolutely needed. Most handlers can be written without
// even initializing it.
//
// The parameters are passed using regparm(2)

__attribute__(( regparm(2) ))
typedef VOID (*FP_IRQ_HANDLR)(VOID);
typedef BYTE RESOURCE_INF;

#define CREATE_IRQ_HANDLER(name) __attribute__(( regparm(2) )) name

typedef enum {
    UNDEFINED = 0,
    BUS_FREE  = 1,  // Available interrupt
    BUS_INUSE = 2,  // 32-bit interrupt controlled by a BUS
    RECL_16   = 3   // Legacy DOS driver interrupt, can be reclaimed
}INTERRUPT_LEVEL;

typedef enum {
    MJ_PNP_UNLOAD,
    MJ_PNP_LOAD,
    MJ_PNP_DISABLE,
    MJ_PNP_ENABLE,
    MJ_REQUEST_INTERRUPT
}PNP_EVENT_CODE;

typedef struct
{
    WORD    major;
    WORD    minor;

}DRIVER_EVENT_PACKET,
*PDRIVER_EVENT_PACKET;

typedef struct {
    PIMUSTR     driver_name;
    PIMUSTR     description;
    PBYTE       cmdline;
    DWORD       driver_flags;
    PVOID       next_driver;
    FP_EVENT_HANDLER event_handler;
}DRIVER_HEADER,*PDRIVER_HEADER;

typedef struct __attribute__((packed))
{
     // Each level is two bits, which means that all interrupt levels fit in
     // a single 32-bit DWORD

    DWORD           lvl_bmp;
    FP_IRQ_HANDLR   handlers[16];
    PDRIVER_HEADER  owners[16];
}INTERRUPTS,
*PINTERRUPTS;

/*
The ISA bus supports 24-bit addresses and up to 16-bit IO ports
Many devices use 10-bit decode.

This is really bad because any address with the bottom bits
reffering to the legacy device will access it, even if the top
bits are something else
*/

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
}IO_RESOURCE,*PIO_RESOURCE;  // IO ports or memory mapped IO


//
// PIO_RESOURCE is MCHUNX because variable interrupts is MCHUNX
//
// An INTERRUPT type is not MCHUNX unless specified, but a pointer to one must
// be because it will certainly point to a MCHUNX INTERRUPT object
//

typedef struct {
    DWORD   signature;
    BYTE    version;
    BYTE    length;
    WORD    control_field;
    BYTE    checksum;
    PVOID   event_notification;
    WORD    _real_mode_code_off;
    WORD    _real_mode_code_seg;
    WORD    protected_off;
    DWORD   protected_base;
    DWORD   oem_device_id;
    WORD    _real_mode_data_seg;
    WORD    protected_data_base;
}*PPNP_INSTALL_CHECK;

extern STATUS APICALL PnAddIOMemRsc(PIO_RESOURCE);
extern VOID InSurrenderInterrupt();
extern VOID InRegainInterrupt();
extern INTERRUPT_LEVEL InGetInterruptLevel(VINT);
extern FP_IRQ_HANDLR InGetInterruptHandler(VINT);
extern STATUS InAcquireLegacyIRQ(VINT, FP_IRQ_HANDLR);
extern VOID InitPnP();
extern __attribute__(( regparm(1) )) WORD PnCallBiosInternal(DWORD argc, ...);


#endif /* PNP_MGR_H */

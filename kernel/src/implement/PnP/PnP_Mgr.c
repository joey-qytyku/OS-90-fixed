/*
     This file is part of OS/90.

    OS/90 is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.

    OS/90 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with OS/90. If not, see <ttps://www.gnu.org/licenses/>.
*/

#include <IA32/PnpSeg.h>

#include <Platform/IO.h>    /* Accessing interrupt mask register */
#include <Misc/Linker.h>    /* Accessing the PnP BIOS structure */
#include <PnP/Core.h>
#include <Debug/Debug.h>
#include <Type.h>

#define PNP_ROM_STRING BYTESWAP(0x24506e50) /* "$PnP" */
#define NUM_INT 16 /* For consistency */

static char driver_name[] = "KERNL386.EXE";
static char description[] = "Kernel plug-and-play support";

//
// Non-standard IRQs are FREE but if they are found
// to have been modified by a DOS program they
// are set to RECL_16
//

////////////////////////////////////////////////////////////////////////////////
// PnP manager functions that modify the interpretation
// of interrupts may not run inside an ISR.
//
// Despite the fact that other driver can modify resource
// information indirectly with function calls, there is no
// need for volatile because that would not be unexpected and
// kernel code cannot be interrupted by a process or driver
//
// But it can now. We will need a critical section for that then. Or not because
// of the global lock.
//
////////////////////////////////////////////////////////////////////////////////

static U32          cur_iorsc = 0;
static IO_RESOURCE  resources[MAX_IO_RSC];
static U16          mask_bitmap = 0xFFFF; // Update this!
static INTERRUPTS   interrupts;

/*
Supported operations with interrupts:

Set operations:
* Request from bus
* Request from kernel bus (legacy)
* Surrender to DOS (change to RECL_16)
* Reclaim from DOS (change to INUSE)

Get operations:
* Get 32-bit handler address
*
*/

////////////////////////////////////////////////////////////////////////////////
// Brief:
//  Get the Int Info object, not for drivers
// v:
//  IRQ number
// U32 to avoid unnecessary sign extention
//
STATUS SetInterruptEntry(
    U32             irq,
    INTERRUPT_CLASS iclass,
    FP_IRQ_HANDLER  handler,
    PDRIVER_HEADER  owner
){

    interrupts.handlers[irq] = handler;
    interrupts.owners[irq] = owner;
    interrupts.class_bmp |= iclass << (irq * 2);

    return OS_OK;
}

//
// A driver or the kernel can voluntarily give an interrupt back to DOS
// Potentially to unload.
//
VOID kernel InSurrenderInterrupt()
{}

INTERRUPT_CLASS kernel InGetInterruptLevel(U32 irq)
{
    return interrupts.class_bmp >>= irq * 2;
}

//
// Get the address of the handler
//
FP_IRQ_HANDLER kernel InGetInterruptHandler(U32 irq)
{
    return interrupts.handlers[irq];
}

// Brief:
//  Legacy IRQ means it the exact IRQ is known by the
//  user and the driver. This function will add a handler
//  and set the interrupt to BUS_INUSE
//
//  If the IRQ is BUS_INUSE, this function fails
//  If BUS_FREE or RECL_16 it is taken
//
// The owner will be the kernel. This does not matter much because the device
// is legacy.
//
STATUS kernel InAcquireLegacyIRQ(
    U32             fixed_irq,
    FP_IRQ_HANDLER  handler
){
    if (InGetInterruptLevel(fixed_irq) == BUS_INUSE)
    {
        return OS_ERROR_GENERIC;
    }

    // If it is a 16-bit interrupt, it is reclaimable, so changing
    // it as a legacy interrupt is correct behavior

    // SetInterruptEntry(
    //     fixed_irq,
    //     BUS_INUSE,
    //     handler,
    //     &kernel_bus_hdr
    // );
}

//
// This API call will reqest the bus driver to give this IRQ to the client.
// Requests to own a bus interrupt are always routed to the particular driver
// for it to handle it how it wants to. Unless it is the kernel,
// in which case, the interrupt is granted if it is BUS_FREE.
//
// This is not the same as requesting control of a device. This will
// simply segment a bus.
//
STATUS kernel InRequestBusIRQ(
    PDRIVER_HEADER  bus,
    PDRIVER_HEADER  client,
    U32             vi,
    FP_IRQ_HANDLER  handler
){
}

//
// Note to self: How will I implement requesting the IRQ based on device ID?
//

VOID kernel PnBiosCall()
{
}

// Scan the ROM space for "$PnP" at a 2K boundary
STATUS SetupPnP(VOID)
{
    // ROM space should not be prefetched or written
    volatile PPNP_INSTALL_CHECK checkstruct = (PPNP_INSTALL_CHECK)0xF0000;
    BOOL supports_pnp;
    U8   compute_checksum;

    // Find the checkstruct
    for (U32 i = 0; i<0x800*32; i++)
    {
        if (checkstruct->signature == PNP_ROM_STRING)
        {
            supports_pnp=1;
            break;
        }
        checkstruct += 0x800; //????
    }
    if (!supports_pnp)
        return OS_FEATURE_NOT_SUPPORTED;

    PnSetBiosDsegBase(checkstruct->protected_data_base);
    PnSetBiosCsegBase(checkstruct->protected_base);

    return OS_OK;
}

////////////////////////////////////////////////////////////////////////////////
// Plug and Play kernel event handling and sending
////////////////////////////////////////////////////////////////////////////////

VOID PnSendDriverEvent()
{
    // Should this require sender != to reciever?
}

//Kernel does not handle events !!!!!!!!!!!!!!! Where to put this?
//
STATUS KernelEventHandler(PDRIVER_EVENT_PACKET)
{
    return OS_FEATURE_NOT_SUPPORTED;
}

////////////////////////////////////////////////////////////////////////////////
// IO and Memory Resource Managment Reoutines
////////////////////////////////////////////////////////////////////////////////

// Add a new port/memory mapped resource entry
//
// This may add additional entries if ISA decode is specified.
//
kernel STATUS PnAddIOMemRsc(PIO_RESOURCE new_rsc)
{
    if (cur_iorsc >= MAX_IO_RSC)
        return -1;
    resources[cur_iorsc] = *new_rsc;
    cur_iorsc++;
    return 0;
}

// Constructor for IO_RESOURCE
kernel VOID NewIOMemRsc(
    PIO_RESOURCE i,
    U32     start,
    U32     size,
    PVOID   owner,
    U16     flags
){
    i->flags = flags;
    i->owner = owner;
    i->start = start;
    i->size  = size;
}

// BRIEF:
//      Allocate IO port space. Returns all ones (-1) if failed. Otherwise
//      returns base IO port.
//
//
kernel U32 PnAllocateIOPorts(U16 num, U8 align)
{
}

//
// Initialization routine. Must only be called by InitPnP
//
// The interrupt mask register is 11111111 for both master and slave
// PIC on startup. When a real mode driver modifies an IRQ vector using,
// INT 21H the IMR is updated by DOS.
//
// If an interrupt is unmasked, it must be assumed that a real mode program
// has inserted an appropriate handler, so it is set to RECL_16. This is how
// legacy interrupts are configured. Windows does this too.
//
// Side note: masking IRQ#2 will mask the entire slave PIC.
//
////// Note on IBM PC Compatiblility Annoyances
//
// By default, the BIOS sends IRQ#9 back to IRQ#2 handler so that a
// program designed for the single PIC thinks a real IRQ#2 happened
// and can use the device.
//
// IRQ#2 never sends any interrupts on the PC-AT architecture.
// If a DOS program hooks onto it
// then OS/90 must ensure that IRQ#9 is blocked off and that IRQ#9
// is handled by the real mode IRQ#2. A protected mode IRQ handler should
// never try to set the IRQ#2 handler, since it will never be called.
//
// This causes a kludge with the master dispatch because IRQ#9 must
// be a legacy 16-bit
// IRQ and be redirected to the IRQ#2 real mode handler. Yuck.
// Try to not use stupid programs plz.
//
static VOID Init_DetectFreeInt(VOID)
{
    U8 imr0 = delay_inb(0x21);

    if (InGetInterruptLevel(2) == RECL_16)
    {
        // Because IRQ#2 == IRQ#9 on PC/AT, both must be blocked off
        // in case of DOS hooking #9. Handler is not real, we are only
        // keeping other drivers from trying to use it.
        InAcquireLegacyIRQ(9, NULL);
    }
}

// Brief:
//  Detect how many COM ports there are and where they
//  are located by reading BDA. The IO base is usually standard
//  but some BIOSes allow ports to be changed.
//
// This required because interrupt levels need to be configured
// even with resource managment turned off
// Notes:
//  COM2 and COM4 => IRQ#3
//  COM1 and COM3 => IRQ#4
//
static VOID DetectCOM(VOID)
{
    U8 i, com_ports;
    const PU16 bda = (PU16)0x400;

    // Check BIOS data area for number of serial ports
    // The beginning words are the COM port IO addresses
    // Zero indictates not present
    //
    for (i = 0; i < 4; i++) // Up to four COM ports on an IBM PC
    {
        if (bda[i] != 0) // then COM[i] exists
            com_ports++;
    }
    // RECL_16 is used because a 32-bit driver for COM is not loaded yet
//    interrupts[4].lvl = RECL_16;


    if (com_ports > 1)
    {
    }
//        interrupts[3].lvl = RECL_16;
}

VOID InitPnP(VOID)
{
    // Clear all interrupt and resource entries. Zeroing them ensures they
    // are recognized as not in use.
    C_memset(&interrupts, 0, sizeof(INTERRUPTS));
    C_memset(&resources,  0, sizeof(IO_RESOURCE) * MAX_IO_RSC);

    Init_DetectFreeInt();
    DetectCOM();
}

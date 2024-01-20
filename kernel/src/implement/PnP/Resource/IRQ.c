///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                     Copyright (C) 2023, Joey Qytyku                       //
//                                                                           //
// This file is part of OS/90 and is published under the GNU General Public  //
// License version 2. A copy of this license should be included with the     //
// source code and can be found at <https://www.gnu.org/licenses/>.          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include <PnP/Resource/IRQ.h>

static U16          mask_bitmap = 0xFFFF; // Update this!
static INTERRUPTS   interrupts;

static STATUS Set_Interrupt_Entry(
    U8       irq,
    ICLASS   iclass,
    ISR      handler,
    P_DRVHDR owner
){
    interrupts.handlers[irq] = handler;
    interrupts.owners[irq] = owner;
    interrupts.class_bmp |= iclass << (irq * 2);

    return OS_OK;
}

kernel VOID Release_IRQ(U8 irq, U32 to)
{
}

kernel ICLASS Get_IRQ_Class(U8 irq)
{
    return interrupts.class_bmp >>= irq * 2;
}

//
// Get the address of the handler
//
kernel ISR Get_IRQ_Handler(U32 irq)
{
    return interrupts.handlers[irq];
}

kernel STATUS Acquire_IRQ(U8  vec, ISR hnd, P_DRVHDR owner)
{
    if (Get_IRQ_Class(vec) == IRQ_INUSE_32)
        return OS_ERROR_GENERIC;

    // If it is a 16-bit interrupt, it is reclaimable, so changing
    // it as a legacy interrupt is correct behavior

    Set_Interrupt_Entry(vec, IRQ_INUSE_32, hnd, owner);
}

// The interrupt mask register is 11111111 for both master and slave
// PIC on startup. When a real mode driver modifies an IRQ vector using,
// INT 21H the IMR is updated by DOS.
//
// If an interrupt is unmasked, it must be assumed that a real mode program
// has inserted an appropriate handler, so it is set to RECL_16. This is how
// legacy interrupts are configured. Windows does this too.
//
// Side note: masking IRQ#2 will mask the entire slave PIC,
//            as is the case on startup
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
static VOID Init_Detect_Free_Int(VOID)
{
    U8 imr0 = delay_inb(0x21);

    if (Get_IRQ_Class(2) == IRQ_RECL_16)
    {
        // Because IRQ#2 == IRQ#9 on PC/AT, both must be blocked off
        // in case of DOS hooking #9. Handler is not real, we are only
        // keeping other drivers from trying to use it.
        Acquire_IRQ(9, NULL, &g_kernel_driver_header);
    }
}

VOID Init_PnP_IRQ()
{}

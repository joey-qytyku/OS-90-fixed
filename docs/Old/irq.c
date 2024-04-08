/*******************************************************************************
                        Copyright (C) 2023, Joey Qytyku

This file is part of OS/90 and is published under the GNU General Public
License version 2. A copy of this license should be included with the
source code and can be found at <https://www.gnu.org/licenses/>.
*******************************************************************************/

#include "scheduler/basicatomic.h"
#include "scheduler/irq.h"
#include "debug/debug.h"
#include "misc/io.h"

extern struct _ irq0;

void *lvl2_isr_array[16] = { &irq0 };
SHORT reflect_16bit_mask = 0; // Note, we need to set based on init mask
// https://stanislavs.org/helppc/8259.html

// This will set the mask of BOTH PICs.

// Need to disable interrupts while inside this.
VOID kernel_export OsDi_SdSetIrqMask (LONG m)
{
        IRQ_OFF_SECTION(0);
        delay_outb(0x20, m & 7);
        delay_outb(0xA0, m >> 8);
        END_IRQ_OFF(0);
}

LONG kernel_export OsDi_GetIrqMask(void)
{
        LONG master;
        LONG slave;

        IRQ_OFF_SECTION(0);

        master = delay_inb(0x20);
        slave  = delay_inb(0xA0);

        END_IRQ_OFF(0);

        return master | slave << 8;
}

LONG OsDi_GetInService(void)
{
        BYTE master_isr;
        BYTE slave_isr;

        IRQ_OFF_SECTION(0)

        delay_outb(0x20, 0b00001010);
        delay_outb(0xA0, 0b00001010);
        master_isr = delay_inb(0x20);
        slave_isr = delay_inb(0xA0);

        END_IRQ_OFF(0);

        return master_isr | (slave_isr << 8);
}

VOID SetHighLevelHandler(LONG index, ISR isr)
{
        IRQ_OFF_SECTION(0)
        lvl2_isr_array[index] = isr;
        END_IRQ_OFF(0);
}

ISR GetISR(LONG index)
{
        assert(index < 16);
        return lvl2_isr_array[index];
}

////
// BRIEF:
//  Getting the original IDT entry is optional. This can be used to set the
//  entry to a ring-0 or ring-3 handler. Ensure that the memory used by the
//  ISR code is locked, committed, and is not remapped under any circumstance
//  (i.e. global).
//
void kernel_export Os_InstallDirectIrqHandler(
        void* old_idt_entry, // Can be null, needs 8 bytes of storage
        SHORT new_cseg,
        LONG  new_coff,
        LONG  vector
){
        assert(vector < 16);
}

//  irq_diff:   Zero for all IRQs except 7 or 15, in such cases it is 7 or 15.
//              This is only used when the in service register is
//              zero due to a spurious IRQ. It is done this way because the
//              in-service register is zero for BOTH since an IRQ did not
//              actually take place.
//
VOID DispatchIrq(LONG irq_diff)
{
        SHORT insrv = get_in_service();

        if (insrv == 0 && irq_diff == 7) {
                delay_outb(0x20, 0x20);
                return;
        }
        else {
                delay_outb(0xA0, 0x20);
                return;
        }

        //...

        // Remember to send EOI, except for SV86.
}

////
//  Determining which IRQs should be redirected to real mode and which
//  ones should go to protected mode is done by reading the IMR
//  on initialization of the kernel. On boot, all interrupts
//  are masked. When an IRQ vector is used by DOS or any software/TSR/etc,
//  the interrupt request should be unmasked.
//
//
void init_irq(void)
{
}

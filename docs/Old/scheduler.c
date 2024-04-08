/*******************************************************************************
                        Copyright (C) 2023, Joey Qytyku

This file is part of OS/90 and is published under the GNU General Public
License version 2. A copy of this license should be included with the
source code and can be found at <https://www.gnu.org/licenses/>.
*******************************************************************************/

#include "scheduler/ALL.h"
#include "debug/debug.h" /* Note: some debug functions may not be safe! */
static long long uptime = 0;

// Note that there are registers that need to be saved, unless
// I use a clobber all asm statement.

atomic_t g_preempt_count;

kernel_export void preempt_dec(void) { K_PreemptDec(); }
kernel_export void preempt_inc(void) { K_PreemptInc(); }

// Can we mask 16-bit IRQs while in SV86? But what if BIOS interrupt need
// the IRQ? Can we nest entering the context? I do not want conditionals.


// We use a separate buffer here because we do not want SV86 to interfere
// with scheduling of tasks. It is a separate mechanism.
// MAY NEED VOLATILE, MEMORY FENCE, OR INTERRUPTS OFF SECTION
static LONG tswitch_restore_context[3]; // ESP, EBP, EIP

////
// BRIEF:
//    IRQ#0 is simple. It just returns to the caller of run_task kind of
//    like setjmp/longjmp.
//
//    Note that in the OS/90 ABI ESI and EDI are call clobbered.
//
void irq0(STDREGS *context)
{
    uptime++;

    debug_log("Timer IRQ recieved\n");

    // Preemption is on, so we can continue the task switch process
    if (atomic_fenced_compare(&g_preempt_count, 0)) {
        context->ESP = tswitch_restore_context[0];
        context->EBP = tswitch_restore_context[1];
        context->EIP = tswitch_restore_context[2];
        return;
    }
}

void run_task(PTASK task)
{
    register LONG esp asm("esp");
    register LONG ebp asm("ebp");

    FENCE(
        tswitch_restore_context[0] = esp;
        tswitch_restore_context[1] = ebp;
        tswitch_restore_context[2] = (Int)&&cont;
    );
    FENCE(
        if (context_currently_in(&task->regs) == IN_KERNEL) {
            enter_kernel_context((pStdregsKernel)&task->regs);
        } else {
            enter_user_context((pStdregsUserGeneric)&task->regs);
        }
    );
    cont:
}
/*******************************************************************************

*******************************************************************************/
LONG OsTsT3_CreateKernelThread(void (*thread_proc)(void))
{
    // ...
}

//
// Sets up SV86 but does not enable interrupts.
//
void init_scheduler_phase0()
{
}

//
// This function is a sort of "shadow" program that controls the scheduler.
// It runs with interrupts disabled so that it can manipulate contexts safely
//
//
_Noreturn static void scheduler_loop(PTASK first)
{
        PTASK c = first;

        while (1) {
                if (c->_flags & TFLAG_MASK == TASK_ON) {
                        for (SHORT i = c->_time_slices; i != 0; i--) {
                                run_task(c);
                        }
                }
                c = c->next;
        }
}

// Configures and enables interrupts
//
void init_scheduler_phase1()
{
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                     Copyright (C) 2023, Joey Qytyku                        //
//                                                                            //
// This file is part of OS/90 and is published under the GNU General Public   //
// License version 2. A copy of this license should be included with the      //
// source code and can be found at <https://www.gnu.org/licenses/>.           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include <scheduler/main.h>
#include <scheduler/sync.h>
#include <scheduler/contexts.h>
#include <scheduler/tss.h>

// SV86 call will clobber general registers and condition codes.
// Only ESP, EBP, and EIP need to be memorized. ESP is in TSS
// so here we just deal with EBP and EIP. These we store in the TSS because
// it shares the same cache line.

static long long uptime;

static atomic_t g_sv86;
static atomic_t g_preempt_count;

// Remember, we do not always want to destroy the current stack.
// Or do we? Not if we yield, of course.
// Add something for saving the context then so that it can be restored.


//  Does not automatically set ESP0 in TSS because this function is not intended
//  to return. In order to implement returning, ESP0 and SS0 must be manually
//  configured so that the stack frame of this function is deleted.
//
//  This may be used for:
//  - Entering SV86
//  - Returning to the process (not for drivers!)
//
// WHAT ABOUT SEGMENT REGISTERS?
_Noreturn
void enter_user_context(struct stdregs *usrCtxPtr)
{
  if (usrCtxPtr->EFLAGS & (1<<17)) {
    BUILD_V86_DSEGS(usrCtxPtr);
  }
  BUILD_RINGSWITCH_STKFRAME(usrCtxPtr);
  LOAD_GENERAL_REGS(usrCtxPtr);
  IRET();
}

void enter_kernel_context(struct stdregs *krnlCtxPtr)
{
  // A kernel context can never be V86. A manual stack change is needed
  // since IRET cannot pop.
  LOAD_GENERAL_REGS(krnlCtxPtr);
  IRET();
}

// Rings 1 and 2 are not supported by OS/90, as seen below.
//
enum context_mode context_currently_in(struct stdregs *anyCtxPtr)
{
  if ((anyCtxPtr->CS & 3) == 0) {
    return IN_KERNEL;
  }
  return IN_USER;
}

kernel void preempt_dec() { K_PreemptDec(); }
kernel void preempt_inc() { K_PreemptInc(); }

// Can we mask 16-bit IRQs while in SV86? But what if BIOS interrupt need
// the IRQ? Can we nest entering the context? I do not want conditionals.

static context_consumer_f sv86_int_handlers[256];
static uint int_counter = 0;

//  SV86 is ONLY for INT calls. Everything else, like far call routines
//  must be based around an INT/IRET. It is the only way for SV86 to operate.
//
// Note: We need to disable interrupts in order to enter a context.
//
void int_sv86(unsigned char vector, struct stdregs *v86CtxPtr)
{
  K_PreemptInc();

  // MUST BE FENCED. enter_user_context depends on the TSS write.

  // Compiler will not eliminate access to the TSS because it is
  // externally linked;

  // Configure TSS values for returning to the caller.
  // We use a goto pointer instead.
  FENCE(
    tss[TSS_SV86_EIP] = (uint)&&cont;
    tss[TSS_SV86_EBP] = _EBP;
    tss[TSS_ESP0]     = _ESP;
  );
  FENCE(enter_user_context(v86CtxPtr));

  cont:

  K_PreemptDec();
}

// We use a separate buffer here because we do not want SV86 to interfere
// with scheduling of tasks. It is a separate mechanism.
static uint tswitch_restore_context[3]; // ESP, EBP, EIP

////
// BRIEF:
//    IRQ#0 is simple. It just returns to the caller of run_task kind of
//    like setjmp/longjmp.
//
//    Note that in the OS/90 ABI ESI and EDI are call clobbered.
//    Use separate context and explain why.
//
//
void irq0(struct stdregs *anyCtx)
{
  uptime++;

  // Preemption is on, so we can continue the task switch process
  if (atomic_fenced_compare(&g_preempt_count,0)) {
    anyCtx->ESP = tswitch_restore_context[0];
    anyCtx->EBP = tswitch_restore_context[1];
    anyCtx->EIP = tswitch_restore_context[2];
    return;
  }
  //
}

void run_task(struct task_desc_blk *task)
{
  FENCE(
    tswitch_restore_context[0] = _ESP;
    tswitch_restore_context[1] = _EBP;
    tswitch_restore_context[2] = &&cont;
  );
  FENCE(
    if (context_currently_in(&task->regs) == IN_KERNEL) {
      enter_kernel_context(&task->regs);
    }
    else {
      enter_user_context(&task->regs);
    }
  );
  cont:


}

//
// Sets up SV86 but does not enable interrupts.
//
void init_scheduler_phase0()
{}

//
// This function is a sort of "shadow" program that controls the scheduler.
// It runs with interrupts disabled so that it can manipulate contexts safely
//
//
_Noreturn
static void scheduler_loop(struct task_desc_blk *first)
{
  struct task_desc_blk *c = first;

  while (1) {
    if (c->_flags & TFLAGS_MASK == TASK_ON) {
      for (uint i = c->time_slices; i >= 0; i--) {
        run_task(c);
      }
    }

    c = c->next;
  }
}

// Configures and enables interrupts
//
void init_scheduler_phase1()
{}

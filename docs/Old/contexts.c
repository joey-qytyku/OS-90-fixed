#include "scheduler/stdregs.h"

// Remember, we do not always want to destroy the current stack.
// Or do we? Not if we yield, of course.
// Add something for saving the context then so that it can be restored.

/*様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様

OS/90 handles the switching of contexts using procedures. Entering a context
is like calling a procedure, but not quite.

The only way to exit a context is to enter a different one or the previous.

This makes the scheduler very flexible and allows interrupts to switch to a
process, yielding to a specific or arbitrary process, and other things.
This also makes the scheduler capable of handling time slices as a simple
for-loop.

IRQ#0 does not switch contexts, but rather, controls returning to the shadow
program.

様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様*/

//
// Remove the condition check? Probably not.
//

VOID NORETURN TsT012Sd_EnterUserContext(PSTDREGS r0context)
{
        if (r0context->EFLAGS & (1 << 17))
                BUILD_V86_DSEGS(r0context);

        BUILD_RINGSWITCH_STKFRAME(r0context);
        LOAD_GENERAL_REGS(r0context);
        IRET();
        // This DOES NOT RETURN, do not listen to compiler
}

VOID NORETURN EnterKernelContext(PSTDREGS r0context)
{
        // A kernel context can never be V86. A manual stack change is needed
        // since IRET cannot pop.
        LOAD_GENERAL_REGS(r0context);
        IRET();
}

BOOL TsTxSd_ContextInKernel(PSTDREGS r0context)
{
        return r0context->CS & 3 == 0;
}

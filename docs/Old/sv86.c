#include "scheduler/basicatomic.h"
#include "scheduler/contexts.h"
#include "scheduler/main.h"
#include "scheduler/sv86.h"

//
// Note that UV86 is the concern of the DOS subsystem.
//

void base_capture(BYTE vector, STDREGS *param)
{
    // I think we need the fallback code here?
    // Or we need a return value to consume the interrupt?
}

static V86Hnd base_sv86_int_capture_ptr = base_capture;
static LONG int_counter = 0;
static ATOMIC32 g_sv86;

/*******************************************************************************

Enters SV86 mode, a non-preemptible and interruptible context in which the
kernel executes a 16-bit real mode INT call in virtual 8086 mode. This will
go through the hook

*******************************************************************************/
kernel_export
void OsT12Sd_Svint(
    BYTE        vector,
    STDREGS*    context
){
    register LONG esp asm("esp");
    register LONG ebp asm("ebp");

    K_PreemptInc();

    // MUST BE FENCED. enter_user_context depends on the TSS write.
    // Compiler will not eliminate access to the TSS because it is
    // externally linked.
    // Configure TSS values for returning to the caller.
    FENCE();
        adwTaskStateSegment[TSS_SV86_EIP] = (LONG)&&cont;
        adwTaskStateSegment[TSS_SV86_EBP] = (LONG)ebp;
        adwTaskStateSegment[TSS_ESP0] = esp;
        context->EFLAGS |= (1<<17);
        atomic_fenced_store(&g_sv86, 1);
    FENCE();

    FENCE();
        enter_user_context(context);
    FENCE();

    cont:

    K_PreemptDec();
}

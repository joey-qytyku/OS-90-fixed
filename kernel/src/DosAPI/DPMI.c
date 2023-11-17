#include <Scheduler/Process.h>
#include <Scheduler/V86M.h>
#include <Scheduler/Sync.h>


// Will be called by scheduler manually. Only an actve process can use 31h,
// so a process control block is used. We need to tell apart 16-bit and 32-bit
// protected mode so the full PCB--not only the registers--will be passed.
VOID HandleInt31h(P_PCB pcb)
{
}

BOOL SV86_DPMI_HandleInt2Fh() { return 0; }

BOOL UV86_DPMI_HandleInt2Fh(BOOL sv86, P_UREGS regs)
{
    // INT 2Fh is hooked by other software besides DPMI.
    // If SV86, we just pass and hope for the best.
    // An active DPMI server will 100% crash the system :)
    // OS/90's bootloader is supposed to handle things like that.

    if (regs->eax == 0x1687) {
        // Installation check
    }
    else if (regs->eax == 0x1686) {
        P_PCB p = GetCurrentPCB();
        if (p->thread_state == THREAD_RUN_PM)
             regs->eax = 0;
        else regs->eax = 1;
    }
}

VOID InitDPMI(VOID)
{
    static V86_CHAIN_LINK int2fh_chain_link = {
        SV86_DPMI_HandleInt2Fh,
        UV86_DPMI_HandleInt2Fh,
        NULL
    };
    HookDosTrap(0x2F, &int2fh_chain_link);
}

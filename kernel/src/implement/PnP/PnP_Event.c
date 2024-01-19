#include <PnP/Event.h>
#include <Scheduler/Sync.h>

static P_MBOX first_mbox;

static VOID Push_Event(
    P_MBOX mb,
    P_EVENT_PACKET ev)
{
    PreemptInc();
    if (mb->stack_top + 1 >= mb->stack_len)
    {
        mb->on_overflow(ev);
        return;
    }

    mb->stack_top++;
    mb->ptr_to_stack[mb->stack_top];
    PreemptDec();
}

// BRIEF:
//      The general purpose event signal routine. This will call the dispatch
//      function of the mailox. All it does is push the event packet to the
//      stack by reference.
//
//      It is possible to raise events within a dispatcher and there are plenty
//      of cases where this is necessary.
//
VOID Raise_Event(
    MBHND           mb,
    P_EVENT_PACKET  ev
){
    Push_Event(mb, ev);
}

// BRIEF:
//      This routine runs every event dispatch function in a loop within a
//      kernel thread. It also runs HLT so that a context switch wakes up the
//      CPU.
//
_Noreturn VOID Event_Dispatch_Thread_Loop(VOID)
{
    P_MBOX mb = first_mbox;
    while (1)
    {
        mb->disp();
        mb = mb->next;
        // Next iteration will be on reschedule
        __asm__ volatile("hlt" ::: "memory");
    }
}

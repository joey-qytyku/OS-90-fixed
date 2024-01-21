////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                     Copyright (C) 2023, Joey Qytyku                        //
//                                                                            //
// This file is part of OS/90 and is published under the GNU General Public   //
// License version 2. A copy of this license should be included with the      //
// source code and can be found at <https://www.gnu.org/licenses/>.           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#if !defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#warning OS/90 requires C99 compiler support.
#endif

#include <Type.h>

#include <Scheduler/Core.h>
#include <IA32/Core.h>

#include <Debug/Debug.h>

#include <Platform/IO.h>

/*
We can make SV86 preemptible.

It can be a strange submode of a kernel thread. It has one shared context
and a lock. When a RECL_16 hits, it modifies this context and pushes stuff
on the stack. Make sure IF=0 on entry.

The SV86 context continues when the thread inside it is scheduled once more.
RECL_16 essentially just schedules an IRQ to be serviced later inside a
non-preemptible context.

If SV86 is inactive (marked with flag), RECL_16 can actually enter it
directly with no problems.

The only change needed is to add segment registers to the kernel context
and maybe an associated flag.

The DOS semaphore is not really useful because it accomplishes nothing that
the SV86 lock does not.  You are not going to have any real concurrency here.
*/

// Does SV86 have to be non-preemptible? Why not just use a lock?
kernel VOID Kernel_Main(VOID)
{
    InitIA32(); // NOT A TRAP GATE

    printf("Hello, world!\n");
}

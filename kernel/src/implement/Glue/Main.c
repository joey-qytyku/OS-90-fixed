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

kernel VOID KernelMain(VOID)
{
    KLogf("Hello, world\n");
}

/*
     This file is part of OS/90.

    OS/90 is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.

    OS/90 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with OS/90. If not, see <https://www.gnu.org/licenses/>.
*/

#if !defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#warning OS/90 requires C99 compiler support.
#endif

#include <Scheduler/Core.h>
#include <IA32/Core.h>

#include <Debug.h>
#include <Type.h>

#include <Platform/IO.h>

PVOID KernelMain(VOID)
{
    PDWORD ivt = (PDWORD)0;

    InitIA32();

    RealModeRegs[0] = 0xE<<8|('A');
    RealModeRegs[1] = 0;

    RealModeTrapFrame[4] = 0x9C00;  // SS
    RealModeTrapFrame[5] = 256;     // ESP
    RealModeTrapFrame[6] = 1<<17 | 3 << 12;   // EFLAGS
    RealModeTrapFrame[7] = ivt[0x10] >> 16; // CS
    RealModeTrapFrame[8] = ivt[0x10] & 0xFFFF; // EIP

    EnterRealMode();
}

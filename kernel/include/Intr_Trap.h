/*
     This file is part of OS/90.

    OS/90 is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.

    OS/90 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with OS/90. If not, see <ttps://www.gnu.org/licenses/>.
*/

#ifndef INTR_TRAP_H
#define INTR_TRAP_H

#include <Type.h>

// For filling the IDT in IA32.c
extern VOID LowDivide0();
extern VOID LowDebug();
extern VOID LowNMI();
extern VOID LowBreakpoint();
extern VOID LowOverflow();
extern VOID LowBoundRangeExceeded();
extern VOID LowInvalidOp();
extern VOID LowDevNotAvail();
extern VOID LowDoubleFault();
extern VOID LowSegOverrun();
extern VOID LowInvalidTSS();
extern VOID LowSegNotPresent();
extern VOID LowStackSegFault();
extern VOID LowGeneralProtect();
extern VOID LowPageFault();
extern VOID LowAlignCheck();

// It would be 17 because vector 15 is reserved
#define EXCEPT_IMPLEMENTED 17

extern VOID Low0();
extern VOID Low1();
extern VOID Low2();
extern VOID Low3();
extern VOID Low4();
extern VOID Low5();
extern VOID Low6();
extern VOID Low7();

extern VOID Low8();
extern VOID Low9();
extern VOID Low10();
extern VOID Low11();
extern VOID Low12();
extern VOID Low13();
extern VOID Low14();
extern VOID Low15();


// In the assembly file, there are instructions for
// each bottom third ISR that save the IRQ number.
// The opcodes: EB 02 6A 08
//
#define BOTTOM_ISR_TABLE_LEN 4

// Defined in Intr_Trap.asm
extern DWORD GetErrCode(VOID);

#endif /* INTR_TRAP_H */

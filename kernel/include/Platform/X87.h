/*
     This file is part of OS/90.

    OS/90 is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.

    OS/90 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with OS/90. If not, see <ttps://www.gnu.org/licenses/>.
*/

// Monitor Co-processor
// If MP and TS are set, the WAIT instruction causes #NM
// (coprocessor/device not available). (also ESC).
// In other words, it decides if FWAIT is affected by the TS bit

#define CR0_MP (1<<1)

// Emulate
// If set, X87 instructions cause #UD so that they may be emulated
// Some kernels use it instead of lazy switching, so that
// FPU registers are saved and restored for a process
// that uses them, OS/90 uses lazy switching instead
#define CR0_EM (1<<2)

// Task switched, not used for software multitasking
// but affects FPU instructions, which cause a #UD so that
// the OS can save them when another task uses the FPU
// The scheduler sets TS to 1 on a context switch 
#define CR0_TS (1<<3)

// Extension type, 80286=0, >=80386=1
// If there is no FPU (EM=1), this bit means nothing
#define CR0_ET (1<<4)

// Native Exception
// If set, exceptions are reported using vector 7
#define CR0_NE (1<<5)

//*The i486DX and better all have integrated floating point
//*units, but they also have the FERR# pin, which is used
//*to report floating point errors from IRQ#13 for compatibility
//*with 80387 code that used this. The IRQ is unavailable for
//*any other purposes
//============================================================
//*The coprocessor slot on i486 boards is simply another socket
//*for an i487, which is a CPU that disables the FPU-less
//*package and takes control of the system instead for i486SX PCs


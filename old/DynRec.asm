

; Not a bit string anymore

;===============================================================================
;    This file is part of OS/90.
;
;   OS/90 is free software: you can redistribute it and/or modify it under the
;   terms of the GNU General Public License as published by the Free Software
;   Foundation, either version 2 of the License, or (at your option) any later
;   version.
;
;   OS/90 is distributed in the hope that it will be useful, but WITHOUT ANY
;   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
;   FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
;   details.
;
;   You should have received a copy of the GNU General Public License along
;   with OS/90. If not, see <https://www.gnu.org/licenses/>.
;===============================================================================

;===============================================================================
;                               D e s c r i p t i o n
;
; The following code implements an optimized dynamic recompiler that
; replaces virtual 8086 mode and translate real mode code into protected mode.
; It runs in ring zero.
;
;                       E n d   o f   D e s c r i p t i o n
;===============================================================================

;===============================================================================
;                                   E q u a t e s
;

        ; Segment selectors for dynrec segments.
GDT_DYN_CS
GDT_DYN_DS
GDT_DYN_SS
GDT_DYN_ES

;
;                            E n d   o f   E q u a t e s
;===============================================================================


;===============================================================================
;    D y n a m i c   R e c o m p i l e r   C o n t e x t   S t r u c t u r e
;

; The machine state structure is as follows:
; EAX
; EBX
; ECX
; EDX
; ESI
; EDI
; EBP
; ESP
; CS
; DS
; ES
; SS
; EFLAGS
;
; Each field is 32-bit.
;
MACH_STATE_SIZE

        absolute 0
DYN_FLAGS               RESD    1
DYN_MACH_STATE          RESD    MACH_STATE_SIZE
DYN_ISP                 RESD    1
DYN_TRACE_BUFF          RESB    128
DYN_VCALL_STACK         RESD    32
DYN_EXEC_FLAGS          RESD    1

        ; If these flags are set, the code segment was modified by a branch
FLAGS_MOD_CS       EQU     0001b

;
;         E n d   o f   D y n R e c   C o n t e x t   S t r u c t u r e
;===============================================================================

        section .text

;===============================================================================
; PARAMS:
;       None, except defaults
;
; OUT: (If copied successfully)
;     EAX := Undefined
;     ECX := 0
; OUT: (If ran out of space)
;     EAX := Undefined
;     ECX := 1
; OUT: (If special instruction found)
;     EAX := Opcode of special instruction (zero extended)
;     ECX := 2
;
FETCH:
        ; We are avoiding unecessary register sharing here.

        xor     eax,eax
        lodsb

        ; Is this a special instruction?
        ; Note that prefixes are handled right here.

        mov     ebx,eax
        mov     ecx,eax

        lea     ebx,[ebx+InsBaseSize]
        lea     ecx,[ebx+UseModrmByteLookup]

        mov     ebx,[ebx]
        mov     ecx,[ecx]

        ; Now we have the size of both components in EBX
        add     ebx,ecx

        ; Get the number of bytes left:
        ; 1. Get the ISP
        ; 2. Remove the base address to get relative byte index
        ; 3. Invert it
        ; 4. Filter the bottom 7 bits.
        ;
        ; Example: The ISP is at 127 relative to trace base address.
        ; 1. ISP := B + 127
        ; 2. ISP - B := 127
        ; 3. ~127 = FFFFFF80h
        ; 4. FFFFFF80h & 1111111b = 0
        ;
        mov     eax,[ebp+DYN_ISP]
        sub     eax,DYN_ISP
        not     eax
        and     eax,1111111b

        ; EAX is the number of bytes left
        ; AND sets ZF

        jz      .no_space

        ; If not taken, we have space for this instruction.

.no_space:

        ret

RETR:
        ret

XWRT:
        ret

        align   16
DynRecMain:


;===============================================================================
;                             D a t a   S e c t i o n
;

        section .data

        ;
        ; Instruction length delta for Mod bits (only those matter here)
        ; RM only selects the registers.
        ;
        align   4
ModLookup:
        DB      2, 1, 2, 0

        ;
        ; How many bytes the instruction takes up including only the opcode,
        ; immediate, and ModRm fields. Prefixes are not included.
        ; 256 bytes total
        ;
        align   4
InsBaseSize:
        %include "OpSizeTab.inc"

        ; A bit string indicating which x86 opcodes use a ModRm byte
        align   4
UseModrmByteLookup:
        %include "ModRmTab.inc"

        ; Opcodes that require special simulation. This is not a lookup table
        ; but an array. There are too few special opcodes that would justify
        ; another lookup table.
SpecialCodes:
        ; TODO: All branches should be here.

NUM_SPECIAL_OPCODES EQU

;
;                    E n d   o f   D a t a   S e c t i o n
;===============================================================================

        section .bss


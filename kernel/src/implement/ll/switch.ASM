; 浜様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様融
; �                   Copyright (C) 2023-2024, Joey Qytyku                     �
; 麺様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様郵
; �  This file is part of OS/90 and is published under the GNU General Public  �
; �    License version 2. A copy of this license should be included with the   �
; �      source code and can be found at <https://www.gnu.org/licenses/>.      �
; 藩様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様夕

        %include "osk/ll/hmadef.inc"

;-------------------------------------------------------------------------------
; This is a procedure that is called by an IRQ handler, but by address rather
;
; Interrupts are assumed to be off here.
; EDX is the interrupt vector to invoke.
;
; All registers are destroyed except ESP.
; SREGS are set to default kernel ones.
;
        bits    16
        xchg    bx,bx

        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳
        ; Calculate the real vector to invoke
        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳

        cmp     dl,7
        ja      .above
        jmp     .else
.above:
        add     edx,70h
.else:
        add     edx,8

        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳
        ; Save stack pointer
        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳
        mov     [SAVE_ESP],esp

        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳
        ; Disable paging and PE.
        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳
        mov     eax,cr0
        and     eax,0x7FFF_FFFE
        mov     cr0,eax

        mov     ax,0FFFFh
        mov     ss,ax
        mov     esp,(SWSTACK-0FFFF0h) + 1024
        mov     ds,ax

        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳
        ; ES will be used to address the IVT
        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳
        xor     ax,ax
        mov     es,ax
        jmp     0xFFFF:SWBASE-0FFFF0h+cont
cont:
        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳
        ; Save current IDTR.
        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳
        sidt    [SWBASE-0FFFF0h+SAVE_IDTR]

        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳
        ; Set IDTR to point to zero with limit 1024
        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳
        lidt    [SWBASE-0FFFF0h+RM_IDTR]

        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳
        ; Simulate INT. Self modifying code is not used.
        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳

        pushfw
        call    FAR [ES:EDX*4]


        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳
        ; Go to protected mode.
        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳

        mov     eax,cr0
        or      eax,8000_0001h
        mov     cr0,eax

        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳
        ; Load valid selectors
        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳

        jmp     8:1_0000h+SWBASE+NEW_CS
NEW_CS:
        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳
        ; Now in 32-bit code segment.
        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳
        bits    32
        mov     eax,10h
        mov     es,ax
        mov     ds,ax
        mov     ss,ax

        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳
        ; Restore stack pointer.
        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳
        mov     esp,[1_0000h+SAVE_ESP]

        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳
        ; Return using a 32-bit size
        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳
        retfd

        align   16
SAVE_ESP:       DD 0
SAVE_IDTR:
        DW      0,0,0,0
RM_IDTR:
        DW      1024,0,0,0
; Wrong order!

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                     Copyright (C) 2022-2024, Joey Qytyku                ;;
;;                                                                         ;;
;; This file is part of OS/90.                                             ;;
;;                                                                         ;;
;; OS/90 is free software. You may distribute and/or modify it under       ;;
;; the terms of the GNU General Public License as published by the         ;;
;; Free Software Foundation, either version two of the license or a later  ;;
;; version if you chose.                                                   ;;
;;                                                                         ;;
;; A copy of this license should be included with OS/90.                   ;;
;; If not, it can be found at <https:;;www.gnu.org/licenses/>              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        %include "OSK/LL/hmadef.inc"

; THIS FILE MUST BE BUILT WITH NASM

;-------------------------------------------------------------------------------
; This is a procedure that is called by an IRQ handler, but by address rather
; than symbol. A 16-bit selector is used.
;
; Interrupts are assumed to be off here.
; EDX is the interrupt vector to invoke.
;
; All registers are destroyed except ESP.
; SREGS are set to default kernel ones.
;
        bits    16
        xchg    bx,bx

        ;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
        ; Calculate the real vector to invoke
        ;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴

        cmp     dl,7
        ja      .above
        jmp     .else
.above:
        add     edx,70h
.else:
        add     edx,8

        ;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
        ; Save stack pointer
        ;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
        mov     [SAVE_ESP],esp

        ;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
        ; Disable paging and PE.
        ;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
        mov     eax,cr0
        and     eax,0x7FFF_FFFE
        mov     cr0,eax

        mov     ax,0FFFFh
        mov     ss,ax
        mov     esp,(SWSTACK-0FFFF0h) + 1024
        mov     ds,ax

        ;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
        ; ES will be used to address the IVT
        ;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
        xor     ax,ax
        mov     es,ax
        jmp     0xFFFF:SWBASE-0FFFF0h+cont
cont:
        ;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
        ; Save current IDTR.
        ;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
        sidt    [SWBASE-0FFFF0h+SAVE_IDTR]

        ;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
        ; Set IDTR to point to zero with limit 1024
        ;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
        lidt    [SWBASE-0FFFF0h+RM_IDTR]

        ;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
        ; Simulate INT. Self modifying code is not used.
        ;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴

        pushfw
        call    FAR [ES:EDX*4]


        ;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
        ; Go to protected mode.
        ;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴

        mov     eax,cr0
        or      eax,8000_0001h
        mov     cr0,eax

        ;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
        ; Load valid selectors
        ;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴

        jmp     8:1_0000h+SWBASE+NEW_CS
NEW_CS:
        ;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
        ; Now in 32-bit code segment.
        ;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
        bits    32
        mov     eax,10h
        mov     es,ax
        mov     ds,ax
        mov     ss,ax

        ;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
        ; Restore stack pointer.
        ;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
        mov     esp,[1_0000h+SAVE_ESP]

        ;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
        ; Return using a 32-bit size
        ;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
        retfd

        align   16
SAVE_ESP:       DD 0
SAVE_IDTR:
        DW      0,0,0,0
RM_IDTR:
        DW      1024,0,0,0
; Wrong order?

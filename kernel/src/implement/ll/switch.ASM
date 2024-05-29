; имммммммммммммммммммммммммммммммммммммммммммммммммммммммммммммммммммммммммммм╩
; ╨                   Copyright (C) 2023-2024, Joey Qytyku                     ╨
; лмммммммммммммммммммммммммммммммммммммммммммммммммммммммммммммммммммммммммммм╧
; ╨  This file is part of OS/90 and is published under the GNU General Public  ╨
; ╨    License version 2. A copy of this license should be included with the   ╨
; ╨      source code and can be found at <https://www.gnu.org/licenses/>.      ╨
; хмммммммммммммммммммммммммммммммммммммммммммммммммммммммммммммммммммммммммммм╪

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

        ;дддддддддддддддддддддддддддддддддддддддддддддддддддддд
        ; Calculate the real vector to invoke
        ;дддддддддддддддддддддддддддддддддддддддддддддддддддддд

        cmp     dl,7
        ja      .above
        jmp     .else
.above:
        add     edx,70h
.else:
        add     edx,8

        ;дддддддддддддддддддддддддддддддддддддддддддддддддддддд
        ; Save stack pointer
        ;дддддддддддддддддддддддддддддддддддддддддддддддддддддд
        mov     [SAVE_ESP],esp

        ;дддддддддддддддддддддддддддддддддддддддддддддддддддддд
        ; Disable paging and PE.
        ;дддддддддддддддддддддддддддддддддддддддддддддддддддддд
        mov     eax,cr0
        and     eax,0x7FFF_FFFE
        mov     cr0,eax

        mov     ax,0FFFFh
        mov     ss,ax
        mov     esp,(SWSTACK-0FFFF0h) + 1024
        mov     ds,ax

        ;дддддддддддддддддддддддддддддддддддддддддддддддддддддд
        ; ES will be used to address the IVT
        ;дддддддддддддддддддддддддддддддддддддддддддддддддддддд
        xor     ax,ax
        mov     es,ax
        jmp     0xFFFF:SWBASE-0FFFF0h+cont
cont:
        ;дддддддддддддддддддддддддддддддддддддддддддддддддддддд
        ; Save current IDTR.
        ;дддддддддддддддддддддддддддддддддддддддддддддддддддддд
        sidt    [SWBASE-0FFFF0h+SAVE_IDTR]

        ;дддддддддддддддддддддддддддддддддддддддддддддддддддддд
        ; Set IDTR to point to zero with limit 1024
        ;дддддддддддддддддддддддддддддддддддддддддддддддддддддд
        lidt    [SWBASE-0FFFF0h+RM_IDTR]

        ;дддддддддддддддддддддддддддддддддддддддддддддддддддддд
        ; Simulate INT. Self modifying code is not used.
        ;дддддддддддддддддддддддддддддддддддддддддддддддддддддд

        pushfw
        call    FAR [ES:EDX*4]


        ;дддддддддддддддддддддддддддддддддддддддддддддддддддддд
        ; Go to protected mode.
        ;дддддддддддддддддддддддддддддддддддддддддддддддддддддд

        mov     eax,cr0
        or      eax,8000_0001h
        mov     cr0,eax

        ;дддддддддддддддддддддддддддддддддддддддддддддддддддддд
        ; Load valid selectors
        ;дддддддддддддддддддддддддддддддддддддддддддддддддддддд

        jmp     8:1_0000h+SWBASE+NEW_CS
NEW_CS:
        ;дддддддддддддддддддддддддддддддддддддддддддддддддддддд
        ; Now in 32-bit code segment.
        ;дддддддддддддддддддддддддддддддддддддддддддддддддддддд
        bits    32
        mov     eax,10h
        mov     es,ax
        mov     ds,ax
        mov     ss,ax

        ;дддддддддддддддддддддддддддддддддддддддддддддддддддддд
        ; Restore stack pointer.
        ;дддддддддддддддддддддддддддддддддддддддддддддддддддддд
        mov     esp,[1_0000h+SAVE_ESP]

        ;дддддддддддддддддддддддддддддддддддддддддддддддддддддд
        ; Return using a 32-bit size
        ;дддддддддддддддддддддддддддддддддддддддддддддддддддддд
        retfd

        align   16
SAVE_ESP:       DD 0
SAVE_IDTR:
        DW      0,0,0,0
RM_IDTR:
        DW      1024,0,0,0
; Wrong order!

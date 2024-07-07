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

%if 0

This file handles IRQs and dispatching system events.

OS/90 does not use software interrupts directly. Every IDT entry is set to
ring-0 in order to make ring-3 emulation of all events possible by causing
a fault every time an INT call or anything like that happens.

Actual exceptions are different and are not actually called by software, but
the CPU and can work with ring-0.

Exceptions are used to emulate everything, if it originates from ring-3,
a chain of handlers is used.

---

IRQs start at vector 0xA0. They are completely reserved vectors that
may not be used for anything ever.

The low-level handlers simply jump to the middle part unless they are
7 or 15, in which case they write those values.



%endif

        %include "osk/ll/hmadef.inc"

        global IDT, IDT_INFO
        global GetInService
        global GetIrqMask
        global SetIrqMask
        global GetStage2ISR
        global SetStage2ISR

        global EXC_0,EXC_1,EXC_2,EXC_3,EXC_4,EXC_5,EXC_6,EXC_7
        global EXC_8,EXC_9,EXC_10,EXC_11,EXC_12
        global EXC_13,EXC_14,EXC_15,EXC_16,EXC_17,EXC_18,EXC_19

        global ISR_7, ISR_15, ISR_REST
        extern SystemEntryPoint

        section .text

%macro Save 0

        push    GS
        push    FS
        push    DS
        push    ES

        push    EBP
        push    EDI
        push    ESI

        push    EDX
        push    ECX
        push    EBX
        push    EAX

%endmacro

%macro Restore 0

        pop     EAX
        pop     EBX
        pop     ECX
        pop     EDX

        pop     ESI
        pop     EDI
        pop     EBP

        pop     ES
        pop     DS
        pop     FS
        pop     GS

%endmacro
        global  Sd_SystemEntryPoint

        align   32
EXC_0:  push    0
EXC_1:  push    1
EXC_2:  push    2
EXC_3:  push    3
EXC_4:  push    4
EXC_5:  push    5
EXC_6:  push    6
EXC_7:  push    7

EXC_8:
        pop     dword [ss:EXC_CODE]
        push    8 ;;
        jmp     EXC_CONT

EXC_9:  push    9
        jmp     EXC_CONT

EXC_10:
        pop     dword [ss:EXC_CODE]
        push    10 ;;
        jmp     EXC_CONT

EXC_11:
        pop     dword [ss:EXC_CODE]
        push    11 ;;
        jmp     EXC_CONT

EXC_12:
        pop     dword [ss:EXC_CODE]
        push    12 ;;
        jmp     EXC_CONT

EXC_13:
        pop     dword [ss:EXC_CODE]
        push    13 ;;
        jmp     EXC_CONT

EXC_14: ; THIS IS BEING CALLED FOR SOME REASON, nah.
        pop     dword [ss:EXC_CODE]
        push    14 ;;
        jmp     EXC_CONT

EXC_15: push    15
        jmp     EXC_CONT

EXC_16: push    16
        jmp     EXC_CONT

EXC_17:
        pop     dword [ss:EXC_CODE]
        push    17 ;;
        jmp     EXC_CONT

EXC_18: push    18
        jmp     EXC_CONT

EXC_19: push    19

        align   16
EXC_CONT:
        Save

        ;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
        ; Call system entry handler.
        ;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
        pop     eax
        mov     edx,[EXC_CODE]
        mov     ecx,esp
        call    SystemEntryPoint

        Restore
        iret

EXC_CODE: DD 0

        align   32
ISR_15:
        mov     BYTE [SS:actual_irq],15
        jmp     ISR_REST

        align   32
ISR_7:
        mov     BYTE [SS:actual_irq],7
        jmp     ISR_REST
        align   32
ISR_REST:
        ; Maybe I should change Ts for thread safe to Tu for threat unsafe.
        Save
        ;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
        ; Get in-service register, but as a bit mask, not an index.
        ;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
        call    GetInService

        xchg    bx,bx

        ;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
        ; Is the in service zero? If so, IRQ is spurious.
        ; Otherwise, continue as usual.
        ; Spurious is unlikely.
        ;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
        test    eax,eax
        jnz     .SPUR

.CONT:
        ;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
        ; This was not a spurious interrupt.
        ;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
        ; Convert the bit mask to an index. It is known that mask != 0.
        ;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
        bsf     ebx,eax

        xchg    bx,bx

        ;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
        ; If the interrupt has no 32-bit handler, reflect to DOS/BIOS
        ;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
        cmp     dword [stage2_isr_list+eax*4],0
        jz      .REFLECT

        ;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
        ; Call interrupt handler and send appropriate EOI. Handlers take the
        ; trap frame pointer. ESP did not change, so pass it directly.
        ;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
        xchg    bx,bx
        mov     eax,esp
        call    [stage2_isr_list+ebx*4]

        ; IRQ index is still in EBX

        ; Are the first 3 bits 0? If so it must be on the slave PIC.
        test    bl,7
        jz      .EOI_BOTH

.REFLECT:
        ;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
        ; Call the reflection entry point in HMA and exit without EOI.
        ; The real mode ISR will send EOI.
        ;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
        xchg    bx,bx
        mov     edx,eax
        call    38h:0
        jmp     .NO_EOI

.SPUR:
        ;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
        ; actual_irq contains the IRQ index since ISR is zero.
        ; If the spurious IRQ came slave, both get the EOI.
        ;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
        cmp     byte [actual_irq],15
        jnz     .NO_EOI

        align   16
.EOI_BOTH:
        mov     al,20h
        out     0A0h,al

        align   16
.EOI_MASTER:
        mov     al,20h
        out     20h,al

        align   16
.NO_EOI:
        Restore
        iret

; TESTED WORKING

SetIrqMask:
        pushf
        cli
        xchg    ah,al
        mov     EBX,EAX
        and     EAX,255
        out     21h,AL

        shr     EBX,8
        mov     EAX,EBX
        out     0A1h,AL
        popf
        ret

; Possibly untested

GetIrqMask:
        pushf
        cli

        xor     eax,eax
        in      al,21h
        mov     ebx,eax

        in      al,0A1h

        shl     ebx,8
        lea     eax,[eax+ebx]

        popf
        ret

        align   64
GetInService:
        pushf
        cli
        xor     eax,eax

        mov     AL,00001010b
        out     20h,AL
        out     0A0h,AL

        in      al,20h
        mov     ebx,eax

        in      al,0A0h

        shl     ebx,8
        or      eax,ebx

        popf
        ret

GetStage2ISR:
        ; Does not need to disable interrupts because it is atomic
        mov eax,[stage2_isr_list+eax*4]
        ret

SetStage2ISR:
        mov [stage2_isr_list+eax*4],eax
        ret

        align   64
;ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ
; Handler for IRQ0. TODO update time in BDA.
;
; This does not deal with inactive tasks.
;
IRQ0:
        ; CONVERT TO A JUMP LOCATION!
        xchg    bx,bx

        xor     ebx,ebx
        add     dword [system_uptime],1
        adc     dword [system_uptime+4],ebx

        cmp     [preempt_count],ebx
        jnz     .SKIP

        ;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
        ; Get the TASK structure.
        ;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
        mov     ebx,esp
        and     ebx,~4095

        ;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
        ; Decrement the scheduler counter in the task.
        ;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
        dec     word [ebx+80+8]

        ;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
        ; If not zero, continue with switching tasks.
        ;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
        jnz     .CONT

        ;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
        ; Reset the counter to its time slice.
        ;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
        mov     ecx,[ebx+80+12]
        mov     [ebx+80+8],ecx

.CONT:
        ; Copy stdregs of the next task and exit the ISR.

        ; If the next task is in V86 mode, we will copy the full 80 bytes
        ; Otherwise, we will copy only 64 bytes.
        xor     eax,eax
        test    byte [ebx+(13*4)+4],(1<<2)      ; ZF = 0 if not V86, 1 if V86
        setz    al                      ; AL = ZF
        shl     eax,4                   ; AL<<4 = 16 or 0
        add     eax,64-4                ; Offset approprately for backward copy

        std
        mov     esi,[ebx+80]    ; Get next pointer in current PCB
        mov     esi,[esi]       ; Dereference the pointer
        add     esi,80-4        ; Make it point to the last register in STDREGS
        lea     edi,[eax+80-4]  ; EDI = Pointer to &stdregs plus size minus 4
        mov     ecx,20
        rep     movsd

        ; I may want to directly fetch the registers at some point.
        ; Would require some stack manipulation, but is doable.

        ;;
        ;; Task state segment?
        ;;

.SKIP:
        ret

;
; Are you sure it's that simple?
; The registers of the userspace are on the stack.
;
; Can we not just put everything on the stack?
;
        align   64
S_Yield:
        ;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
        ; Take away all time slices from current task.
        ;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

        cli
        mov     eax,esp
        and     eax,~4095
        mov     word [eax+80+8],0

        ;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
        ; Decrement the system uptime counter to compensate.
        ;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

        sub     dword [system_uptime],1
        sbb     dword [system_uptime+4],0

        int     0A0h
        sti
        ret

        section .data
stage2_isr_list:
        DD      IRQ0
        DD      0,0,0,0,0
        DD      0,0,0,0,0
        DD      0,0,0,0,0

IDT_INFO:
        DW      2048
        DD      IDT

        section .bss

        align   16
IDT:
        RESB    2048


preempt_count RESD 0

system_uptime RESQ 0

actual_irq RESB 0
        ; TODO

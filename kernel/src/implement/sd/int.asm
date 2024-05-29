; ����������������������������������������������������������������������������ͻ
; �                   Copyright (C) 2023-2024, Joey Qytyku                     �
; ����������������������������������������������������������������������������͹
; �  This file is part of OS/90 and is published under the GNU General Public  �
; �    License version 2. A copy of this license should be included with the   �
; �      source code and can be found at <https://www.gnu.org/licenses/>.      �
; ����������������������������������������������������������������������������ͼ

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

        %include "lowlevel/hmadef.inc"

        global IDT, IDT_INFO
        global OsDiSd_GetInService
        global OsDiSd_GetIrqMask
        global OsDiSd_SetIrqMask
        global OsDiTxSd_GetStage2ISR
        global OsDiTxSd_SetStage2ISR

        global EXC_0,EXC_1,EXC_2,EXC_3,EXC_4,EXC_5,EXC_6,EXC_7,EXC_8,EXC_9,EXC_10,EXC_11,EXC_12,EXC_13,EXC_14,EXC_15,EXC_16,EXC_17,EXC_18,EXC_19

        global ISR_7, ISR_15, ISR_REST
        extern Sd_SystemEntryPoint
        extern printf_
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

EXC_14: ; THIS IS BEING CALLED FOR SOME REASON
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

        ;�����������������������������������������������������������������������
        ; Call system entry handler.
        ;�����������������������������������������������������������������������
        pop     eax
        mov     edx,[EXC_CODE]
        mov     ecx,esp
        call    Sd_SystemEntryPoint

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
        ;�����������������������������������������������������������������������
        ; Get in-service register, but as a bit mask, not an index.
        ;�����������������������������������������������������������������������
        call    OsDiSd_GetInService

        ; I don't think this is right...
        ; It gives me the B8_8F thing.

        xchg    bx,bx

        ;�����������������������������������������������������������������������
        ; Is the in service zero? If so, IRQ is spurious.
        ; Otherwise, continue as usual.
        ; Spurious is unlikely.
        ;�����������������������������������������������������������������������
        test    eax,eax
        jz      .SPUR

.CONT:
        ;�����������������������������������������������������������������������
        ; This was not a spurious interrupt.
        ;�����������������������������������������������������������������������
        ; Convert the bit mask to an index. It is known that mask != 0.
        ;�����������������������������������������������������������������������
        bsf     eax,eax

        xchg    bx,bx

        ;�����������������������������������������������������������������������
        ; If the interrupt has no 32-bit handler, reflect to DOS/BIOS
        ;�����������������������������������������������������������������������
        cmp     dword [stage2_isr_list+eax*4],0
        jz      .REFLECT

        ;�����������������������������������������������������������������������
        ; Call interrupt handler and send appropriate EOI. Handlers take the
        ; trap frame pointer. ESP did not change, so pass it directly.
        ;�����������������������������������������������������������������������
        xchg    bx,bx
        mov     eax,esp
        lea     ebx,[stage2_isr_list+eax*4]
        call    ebx

        ; IRQ index is still in EBX

        ; Are the first 3 bits 0? If so it must be on the slave PIC.
        test    bl,7
        jz      .EOI_BOTH

.REFLECT:
        ;�����������������������������������������������������������������������
        ; Call the reflection entry point in HMA and exit without EOI.
        ; The real mode ISR will send EOI.
        ;�����������������������������������������������������������������������
        xchg    bx,bx
        mov     edx,eax
        call    38h:0
        jmp     .NO_EOI

.SPUR:
        ;�����������������������������������������������������������������������
        ; actual_irq contains the IRQ index since ISR is zero.
        ; If the spurious IRQ came slave, both get the EOI.
        ;�����������������������������������������������������������������������
        cmp     byte [actual_irq],15
        jnz     .NO_EOI

.EOI_BOTH:
        mov     al,20h
        out     0A0h,al
.EOI_MASTER:
        mov     al,20h
        out     20h,al
.NO_EOI:
        Restore
        iret

; TESTED WORKING

OsDiSd_SetIrqMask:
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

OsDiSd_GetIrqMask:
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

; PROBLEM: NEXT READ IS FROM FIRST PORT, NOT SECOND. THe IMR is always the second.
OsDiSd_GetInService:
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

OsDiTxSd_GetStage2ISR:
        ; Does not need to disable interrupts because it is atomic
        mov eax,[stage2_isr_list+eax*4]
        ret

OsDiTxSd_SetStage2ISR:
        mov [stage2_isr_list+eax*4],eax
        ret

;�������������������������������������������������������������������������������
; Handler for IRQ0. TODO update time in BDA.
;
IRQ0:
        xchg    bx,bx

        xor     ebx,ebx
        add     dword [system_uptime],1
        adc     dword [system_uptime+4],ebx

        cmp     [preempt_count],ebx
        jnz     .SKIP


        ;�����������������������������������������������������������������������
        ; Get the TASK structure.
        ;�����������������������������������������������������������������������
        mov     ebx,esp
        and     ebx,~4095


        ;�����������������������������������������������������������������������
        ; Decrement the scheduler counter in the task.
        ;�����������������������������������������������������������������������
        dec     word [ebx+80+4]

        ;�����������������������������������������������������������������������
        ; If not zero, continue with switching tasks.
        ;�����������������������������������������������������������������������
        jnz     .CONT

        ;�����������������������������������������������������������������������
        ; Reset the counter to its time slice.
        ;�����������������������������������������������������������������������
        mov     ecx,[ebx+80+8]
        mov     [ebx+80+4],ecx

.CONT:
        ; Copy stdregs of the next task and exit the ISR.
        std
        mov     esi,[ebx+80]    ; Get next pointer in current PCB
        mov     esi,[esi]       ; Dereference the pointer
        add     esi,80-4        ; Make it point to the last register in STDREGS
        lea     edi,[eax+80-4]  ; EDI = Pointer to &stdregs plus size minus 4
        mov     ecx,20
        rep     movsd

.SKIP:
        ret

OsT12Sd_Yield:
        ;�����������������������������������������������������������������������
        ; Take away all time slices from current task.
        ;�����������������������������������������������������������������������

        mov     eax,esp
        and     eax,~4095
        mov     word [eax+80+4],0

        ;�����������������������������������������������������������������������
        ; Decrement the system uptime counter to compensate.
        ;�����������������������������������������������������������������������

        sub     dword [system_uptime],1
        sbb     dword [system_uptime+4],0

        ;�����������������������������������������������������������������������
        ; Call the IRQ0 handler (can I do better though?)
        ;�����������������������������������������������������������������������

        cli
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

%if 0

This file handles IRQs and dispatching system events.

OS/90 does not use software interrupts directly. Every IDT entry is set to
ring-0 in order to make ring-3 emulation of all events possible by causing
a fault every time an INT call or anything like that happens.

Actual exceptions are different and are not actually called by software, but
the CPU and can work with ring-0.

Exceptions are used to emulate everything, if it originates from ring-3.
A chain of handlers is used.

---

IRQs start at vector 0xA0. They are completely reserved vectors that
may not be used for anything ever.

The low-level handlers simply jump to the middle part unless they are
7 or 15, in which case they write those values.



%endif

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
        Save

        ;----------------------------------------------------------
        ; Get in-service register, but as a bit mask, not an index.
        ;----------------------------------------------------------
        call    OsDiSd_GetInService

        ;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
        ; Is the in service zero? If so, IRQ is spurious.
        ; Otherwise, continue as usual.
        ; Spurious is unlikely.
        ;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
        test    eax,eax
        jz      .SPUR

        ;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�
        ;
        ;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�

.CONT:

.SPUR:
        ;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
        ; actual_irq contains the IRQ index since ISR is zero.
        ; If the spurious IRQ came slave, both get the EOI.
        ;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴


        mov     al,20h
        out     20h,al

        cmp     byte [actual_irq],15
        jnz     .END
        out     0A0h,al

.END:
        Restore
        iret


OsDiSd_SetIrqMask:
        pushf
        cli
        mov     EBX,EAX
        and     EAX,7
        out     20h,AL

        shr     EBX,8
        mov     EAX,EBX
        out     0A0h,AL
        popf
        ret

OsDi_GetIrqMask:
        pushf
        cli

        xor     eax,eax
        in      al,20h
        mov     ebx,eax

        in      al,0A0h

        shl     ebx,8
        lea     eax,[eax+ebx]

        popf
        ret

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
        lea     eax,[eax+ebx]

        popf
        ret

        section .data

        section .bss

actual_irq RESB 0

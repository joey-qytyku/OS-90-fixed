extern TSS

global EXC_0

global ISR_1, IRQ0

global EXC_0, ISR_REST, IRQ0, RemapPIC, SetIrqMask
global V86_Monitor, INTxH, EnterV86

global handler_table

_EAX    EQU 0   * 4
_EBX    EQU 1   * 4
_ECX    EQU 2   * 4
_EDX    EQU 3   * 4
_ESI    EQU 4   * 4
_EDI    EQU 5   * 4
_EBP    EQU 6   * 4
_pm_ES  EQU 7   * 4
_pm_DS  EQU 8   * 4
_pm_FS  EQU 9   * 4
_pm_GS  EQU 10  * 4
_EIP    EQU 11  * 4
_CS     EQU 12  * 4
_EFLAGS EQU 13  * 4
_ESP    EQU 14  * 4
_SS     EQU 15  * 4
_v86_ES EQU 16  * 4
_v86_DS EQU 17  * 4
_v86_FS EQU 18  * 4
_v86_GS EQU 19  * 4
_tb_size EQU 20  * 4
_next EQU _tb_size

_switch_action equ 21 * 4

_tss_ESP0 EQU 4
_tss_SS0 EQU 8

        ; I think IRET is IOPL sensitive and it is returning to the wrong place.

%macro Save 0
        push    gs
        push    fs
        push    ds
        push    es

        push    ebp
        push    edi
        push    esi

        push    edx
        push    ecx
        push    ebx
        push    eax
%endmacro

%macro Restore 0
        pop     eax
        pop     ebx
        pop     ecx
        pop     edx

        pop     esi
        pop     edi
        pop     ebp

        pop     es
        pop     ds
        pop     fs
        pop     gs
%endmacro

%macro Exc_nocode 1
        align   16
        global EXC_%+%1

EXC_%+%1:
        push %1
        jmp EXC_CONT
%endmacro

%macro Exc_code 1
        align   16
        global EXC_%+%1
EXC_%+%1:
        pop     dword [ss:EXC_CODE]
        push    %1
        jmp     EXC_CONT
%endmacro

section .text

Exc_nocode 0
Exc_nocode 1
Exc_nocode 2
Exc_nocode 3
Exc_nocode 4
Exc_nocode 5
Exc_nocode 6
Exc_nocode 7
Exc_code   8
Exc_nocode 9
Exc_code   10
Exc_code   11
Exc_code   12
Exc_code   13
Exc_code   14
Exc_nocode 15
Exc_nocode 16
Exc_code   17
Exc_nocode 18
Exc_nocode 19

; Put in the rest?

; Note, current layout for exceptions is technically not reentrant.
; I need to change that later. Error code should be passed on stack.

        align   64
EXC_CONT:
        pop     dword [ss:EXC_INX]
        Save

        mov     eax,10h
        mov     ds,ax
        mov     es,ax

        ; If the stage-2 handler is written in assembly, it can use EDX
        ; to access the saved registers.

        mov     edx,esp

        ; Interrupts are currently off, enable
        ; sti

        ; xchg bx,bx

        ;;;;;;;;;;;;; Problem probably here.
        ; V86 monitor barely changed. Entrace process is correct and has no
        ; errors. I think that the return process is wrong and it has something
        ; to do with the fact that we bypass the entire exception return
        ; mechanism.

        ; Especially not the argument being pushed along with the return addr.

        push    edx
        mov     ebx,[EXC_INX]
        call    [handler_table+ebx*4]
        add     esp,4

        Restore
        iret

        align 4
EXC_CODE DD 0
EXC_INX  DD 0

        align 4
        ; Set the handler? It is not set!
handler_table:
        times 20 DD 0FFFF_FFFFh

;============================================================================
; IRQ Related Routines
;============================================================================

; TODO, save ebx

        align   16
ISR_1:  mov     byte [ss:actual_irq],1
        jmp     ISR_REST

        align   16
ISR_2:  mov     byte [ss:actual_irq],2
        jmp     ISR_REST

        align   16
ISR_3:  mov     byte [ss:actual_irq],3
        jmp     ISR_REST

        align   16
ISR_4:  mov     byte [ss:actual_irq],4
        jmp     ISR_REST

        align   16
ISR_5:  mov     byte [ss:actual_irq],5
        jmp     ISR_REST

        align   16
ISR_6:  mov     byte [ss:actual_irq],6
        jmp     ISR_REST

        align   16
ISR_7:
        mov     byte [ss:actual_irq],7
        jmp     ISR_REST

ISR_8:  mov     byte [ss:actual_irq],8
        jmp     ISR_REST

        align   16
ISR_9:  mov     byte [ss:actual_irq],9
        jmp     ISR_REST

        align   16
ISR_A:  mov     byte [ss:actual_irq],10
        jmp     ISR_REST

        align   16
ISR_B:  mov     byte [ss:actual_irq],11
        jmp     ISR_REST

        align   16
ISR_C:  mov     byte [ss:actual_irq],12
        jmp     ISR_REST

        align   16
ISR_D:  mov     byte [ss:actual_irq],13
        jmp     ISR_REST

        align   16
ISR_E:  mov     byte [ss:actual_irq],14
        jmp     ISR_REST

        align   16
ISR_F:  mov     byte [ss:actual_irq],15
        jmp     ISR_REST

        align   16
ISR_REST:
        push    ds
        Save
        mov     eax,10h
        mov     ds,ax
        ; Save the other segments?

        ; TODO: Handle spurious interrupts

.CONT:
        ;--------------------------------------------------------------------
        ; This was not a spurious interrupt.
        ;--------------------------------------------------------------------

        mov     ebx,[actual_irq]

        ;--------------------------------------------------------------------
        ; If the interrupt has no 32-bit handler, reflect to DOS/BIOS
        ;--------------------------------------------------------------------
        cmp     dword [stage2_isr_list+ebx*4],0
        jz      .REFLECT

        ;--------------------------------------------------------------------
        ; Call interrupt handler and send appropriate EOI. Handlers take the
        ; trap frame pointer. ESP did not change, so pass it directly.
        ;--------------------------------------------------------------------
        mov     eax,esp
        call    [stage2_isr_list+ebx*4]

        ; IRQ index is still in EBX

        ; Are the first 3 bits 0? If so it must be on the slave PIC.
        test    bl,7
        jz      .EOI_BOTH

.REFLECT:
        ;--------------------------------------------------------------------
        ; Call the reflection entry point in HMA and exit without EOI.
        ; The real mode ISR will send EOI.
        ;--------------------------------------------------------------------
        mov     edx,ebx
        call    28h:16+3000h
        ; That just trashed all the registers, but we dont care now.
        jmp     .NO_EOI

.SPUR:
        ;--------------------------------------------------------------------
        ; actual_irq contains the IRQ index since ISR is zero.
        ; If the spurious IRQ came slave, both get the EOI.
        ;--------------------------------------------------------------------
        cmp     BYTE [actual_irq],15
        jnz     .NO_EOI

        align   16
.EOI_BOTH:
        mov     al,20h
        out     0A0h,al
	jmp $+2

.EOI_MASTER:
        mov     al,20h
        out     20h,al
	jmp $+2

.NO_EOI:
        Restore
        pop     ds

        iret

        align 4
actual_irq: DD 0

        align   16
stage2_isr_list:
        DD      IRQ0
        times 15 DD 0


%include "a1swtask.inc"
%include "a1sv86.inc"

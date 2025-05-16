
; A two-dimensional array is used to index the approprate task switch handler.
; I can use E9 to debug this.

; I am not sure if branch avoidance is the perfect strategy. I still have to
; deal with time slices.

section .text

;
; Switch to 16-bit offsets. The assembler can make this work and I tested it
; for COFF output.
;
; goto [current.index+next.index*2 + next.index + table_base]
;

;
; A matrix of possible switch actions to be taken.
; The Y-axis it the source. The X-axis is the destination.
;
align 32
sw:
;       To R0      To R3      To VM
;----------------------------------------;
DD      r0_r0,     r0_r3,     r0_vm,0 ; From R0
DD      r3_r0,     r3_r3,     r3_vm,0 ; From R3
DD      vm_r0,     vm_r3,     vm_vm,0 ; From VM

; Changing things in the TB is potentially dangerous from C because
; the compiler does not know that the variables may be used later by the IRQ,
; especially when the pointer is not passed around.

	align   32
IRQ0:
        xchg bx,bx;;;;

        push    ebx
        push    ecx

        mov     ebx,esp
        and     ebx,~4095

        mov     ecx,[ss:ebx+_next]

        ; We need the data segment to be set. This avoids copying a stack
        ; segment override over 7 times.
        mov     [ss:ebx+_pm_DS],ds

        ; Replace with a load?
        push    byte 8
        pop     ds

        ; Save all general purpose registers. This faster than pushing because
        ; there is no address generation interlock, but the imm has 1 clock
        ; penalty on 486. Not on 586 though.
        mov     [ebx+_EAX],eax
        mov     [ebx+_EDX],edx
        mov     [ebx+_ESI],esi
        mov     [ebx+_EDI],edi
        mov     [ebx+_EBP],ebp

        pop     dword [ebx+_ECX]
        pop     dword [ebx+_EAX]

        ; Get both of the switch actions.
        mov     esi,[ebx+_switch_action]
        mov     edi,[ecx+_switch_action]

        shl     edi,4
        jmp     [esi*4+edi+sw]

        align   32
r0_r0:
        ; The only thing left to do is to get into the context by loading
        ; the registers. EBX is still the current task and ECX is the
        ; target. We only need ECX of course.

        ; The only flags that matter in a r0-r0 transfer of state are the
        ; regular integer arithmetic ones and IF. popfd is actually fully
        ; effective. No need for an IRET. There is also no code change to the
        ; code segment, and sreg loads are wasteful in this case.



        mov     esp,[ecx+_ESP]

        align   32
r0_r3:

r0_vm:

r3_r0:
r3_vm:
r3_r3:

vm_r0:
vm_r3:
vm_vm:

current_task:   DD 0 ; Remove this?
system_uptime:  DQ 0
preempt_count:  DD 0


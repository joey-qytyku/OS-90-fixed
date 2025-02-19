
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
DW      r0_r0-sw,  r0_r3-sw,  r0_vm-sw,0 ; From R0
DW      r3_r0-sw,  r3_r3-sw,  r3_vm-sw,0 ; From R3
DW      vm_r0-sw,  vm_r3-sw,  vm_vm-sw,0 ; From VM

	align   32
IRQ0:
        xchg bx,bx;;;;

        push    ebx
        push    ecx

        mov     ebx,esp
        and     ebx,~4095

        ; With the pointer to the current task RD in EBX, calculate the address
        ; of the next task in ECX.
        ;
        ; Alternating between two registers could improve performance, but
        ; I will not try that for now.

        mov     ecx,[ss:ebx+_next]

        ; We need the data segment to be set. This avoids copying a stack
        ; segment override over 7 times.
        mov     [ss:ebx+_DS],ds

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

        ; All registers are now copied. EBX and ECX still the same.
        ; Get both of the switch actions.
        mov     esi,[ebx+_switch]
        mov     edi,[ecx+_switch]

        ; Load SOME of the next task registers. EAX, EBX, and ECX must be done
        ; manually.

        jmp     [esi*2+edi+sw]

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

current_task:   DD 0
system_uptime:  DQ 0
preempt_count:  DD 0


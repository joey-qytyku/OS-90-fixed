global preempt_count

; A two-dimensional array is used to index the approprate task switch handler.
; I can use E9 to debug this.

; I am not sure if branch avoidance is the perfect strategy. I still have to
; deal with time slices.

section .text

align 8
sw:

; Changing things in the TB is potentially dangerous from C because
; the compiler does not know that the variables may be used later by the IRQ,
; especially when the pointer is not passed around.
; Honestly, I should write the whole scheduler in ASM if that is a problem.

; BTW add an IRQ0 for preemption disabled?

	align   32
IRQ0:
        xchg bx,bx;;;;


        push    ebx

        ; EBX = Current task
        mov     ebx,esp
        and     ebx,~(4095)

        ; Save EBX to the register dump (SS needed?)
        pop     dword [ss:ebx+_EBX]

        ; Save data segment so we can stop using overrides
        ; In the case of V86, the data segment is already saved so this value
        ; we put there won't matter.
        ; Change to new DS.
        mov     [ss:ebx+_EAX],eax
        mov     [ss:ebx+_pm_DS],ds

        mov     eax,8
        mov     ds,ax

        mov     [ebx+_ECX],ecx
        mov     [ebx+_EDX],edx
        mov     [ebx+_ESI],esi
        mov     [ebx+_EDI],edi
        mov     [ebx+_EBP],ebp

        jmp     [ebx+_switch_action]

system_uptime:  DQ 0
preempt_count:  DD 0


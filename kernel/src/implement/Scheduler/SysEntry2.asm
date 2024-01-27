;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                           ;;
;;                     Copyright (C) 2023, Joey Qytyku                       ;;
;;                                                                           ;;
;; This file is part of OS/90 and is published under the GNU General Public  ;;
;; License version 2. A copy of this license should be included with the     ;;
;; source code and can be found at <https://www.gnu.org/licenses/>.          ;;
;;                                                                           ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "Macro.inc"
%include "Scheduler/SchedDefs.inc"

;===============================================================================
; S y s t e m   E n t r y   I m p l e m e n t a t i o n   i n   A s s e m b l y
;===============================================================================

;===============================================================================
;                               I m p o r t s
;===============================================================================

extern System_Exit_Point, _dwErrorCode

IMPORT Segment_Util, Svint86

;===============================================================================
;                         E n d   o f   I m p o r t s
;===============================================================================

;===============================================================================
;                               E q u a t e s
;===============================================================================

; TODO: Reorder for call table use
; Also move into Structures.inc
LOCAL_PM_INT_REFLECT_GLOBAL equ 0
LOCAL_PM_INT_TRAP           equ 3
LOCAL_PM_IRQ_REFLECT_GLOBAL equ 1
LOCAL_PM_INT_IDT_FAKE_IRQ   equ 2
LOCAL_V86_INT_REFLECT       equ 4
LOCAL_PM_EXCEPTION          equ 5

;===============================================================================
;                        E n d   o f   E q u a t e s
;===============================================================================

;===============================================================================
;                           S t r u c t u r e s
;===============================================================================

;
; When entering, we push the segment registers onto the stack in all cases.
; If it is V86, zero sregs are pushed and later popped with no problems.
; If PM, the actual sregs are pushed.
;
; The difficulty is that the IRET frame is variably sized, since only V86
; pushes sregs. (Intel should have made all segment registers save on trap.)
;
; This means that copying to-from a process control block requires checking
; the VM flag to know how much data to copy.

; How many IRET do operations will I need?

;===============================================================================
;                    E n d   o f   S t r u c t u r e s
;===============================================================================

;-------------------------------------------------------------------------------
; This will be called in T2 to emulate an INT instruction in a V86
; process.
;
; This must be called only when the process is in V86 mode and cannot be used
; in the event of a PM INT reflection.
;
; A fake IRQ to a V86 process will jump to this
;

Int_Sep_Do_V86:

;-------------------------------------------------------------------------------
; This will be called in T2.
;
Int_Sep_Do_PM:
        ; We HAVE to get the interrupt vector. This requires getting the
        ; CS:EIP first. There is a convenient function for getting the
        ; segment base address

        INVOKE Segment_Util, 2

        ; Add saved EIP to it. For traps, the EIP points after the instruction
        ; so we subtract one to get the imm8 byte
        add     eax,[ebp+RD._eip-1]     ; IFRAME is first in PCB
        cmp     U32 [eax-1],0CDh
        jnz     .not_an_INT_opcode
        mov     eax,[eax]                       ; Load the imm8 in EAX

        ; We need to check the virtual IDT and handle each case.

        push    eax
        mov     ecx,[ebp+PCB.local_idt]

        imul    eax,6   ; Each LIDT entry is 6 bytes
        add     ecx,eax
        pop     eax

        ; Check the type field.
        ; It must be an actual trap. Not an exception or IRQ If reflection
        ; we fall back to V86 sepdo

        ;------------------------------------------------
        ; EAX := Index of IDT entry (will discard soon)
        ; ECX := Address of LIDT entry

        mov     [.vector],al

        ; Could optimize with branch table.
        test    [ecx+4],LOCAL_PM_INT_TRAP<<13
        jz      .do_call_local_handler ; sure?
        test    [ecx+4],LOCAL_PM_INT_REFLECT_GLOBAL<<13
        jz      .do_reflect_to_LIVT
        jmp     .not_an_INT_opcode     ; TEST is AND op, so zero means not equal

.table: ; Working on this
        DD      .do_call_local_handler
        DD      .do_reflect_to_LIVT
        DD      .do_reflect_to_SV86

.do_call_local_handler:
        ; Otherwise, we are clear to switch control flow to the handler.
        ; INT calls in DPMI use far call frames and must be accurately emulated.

        ; Get the segment and offset field of the entry
        mov     eax,[ecx+4]
        and     ax,~(1<<13)
        mov     ebx,[ecx]

        ; EAX:EBX := Seg:off of the stack
        ; ECX := Address of LIDT entry

        ; Two ways this can happen:
        ; * 16-bit PM
        ; * 32-bit PM
        ;
        ; What happens next is determined by the CSEG size of the process.
        ;
        ; We will get clever with the 32-bit one and change the stack pointer
        ; so that we can just push the values with no mem copy.

        lar     dx,ax
        jc      .bits_16

._bits_32:
        ; We do not want to have an interrupt frame inside a user context,
        ; so disable IRQs.
        cli
        mov    di,ss
        mov    si,esp

        ; This looks pointless, but it avoids a lot more memory transfers
        ; to unscramble descriptor addresses and calculate the absolute.

        ; Load SS:ESP from the PCB like a far pointer using LSS
        ; Why from the PCB and not the stack frame?
        lss     esp,[ebp+RD._esp]

        ;---------------------------------
        ; Register recap:
        ; EAX    := Intended CS
        ; EBX    := Intended EIP
        ; SS:ESP := EAX:EBX

        ; Generate RETF frame. It needs to work with an actual RETF instruction.
        ; Far call saves the stack pointer for inter-privilege calls and this is
        ; required by DPMI. However, this call is not actually inter-privilege
        ; in OS/90. We do not care though. Software that modifies the stack
        ; to chain or does other things will NOT work.

        push    ebx     ; CS
        push    eax     ; EIP

        mov     ss,di
        mov     esp,esi
        sti

        ; Done
        jmp     System_Exit_Point

._bits_16:
        ; Only a 16-bit code segment can push 16-bit values.
        ; To do it anyway, we will bitwise shift and or them together
        ; { TODO }
        jmp     System_Exit_Point

.do_reflect_to_SV86:
        ; There is no hook for real mode reflection or protected mode handling.
        ; Must use Svint86 to service.
        ; The register parameters of Svint86 will be changed to RD,
        ; so we can call it with whatever is in EBP.
        push    ebp
        push    byte [.vector]
        call    Svint86

        ; Svint86 saves results to provided buffer, in this case, process
        ; registers. Now done.
        jmp     System_Exit_Point

.do_reflect_to_LIVT:
        ; Reflect to real mode. This will be terminated by an IRET
        ; instruction and will have an IRET stack frame.

        ; We will switch the process to virtual 8086 mode.
        jmp     System_Exit_Point

.no_trap_handler:
        ; An INT has been invoked on a non-INT LIDT entry
        ; This is an error. Process must terminate.

.not_an_INT_opcode:
        ; Operation was not an INT. This is an error.

        ; Remeber DPMI exceptions turn off virtual IRQs.

.vector DB 0

; But could we just call the far call handler with user selector?
; 4 except, not int.


;-------------------------------------------------------------------------------
; INPUT:
;       EDX = Event code
;       ESP = Last value pushed to trap frame
;
; If the event code is between 0-31, it is an exception.
; If the event code is 255, INT call
;
System_Entry_Point:
        ; Set EBP to current PCB for future use
        mov     ebp,esp
        and     ebp,~1FFFh

        cmp     dl,255
        jnz     .exception

        ; It was an INT
        ; Maybe do not use a call?
        jmp     [ebp+PCB.sepdo_int]

.exception:

        jmp     System_Exit_Point

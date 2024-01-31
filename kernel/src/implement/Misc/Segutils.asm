%include "Macro.inc"

;===============================================================================
;                               I m p o r t s
;===============================================================================

extern aqwLocalDescriptorTable

IMPORT IaGetBaseAddress
IMPORT IaAppendLimitToDescriptor
IMPORT IaAppendAddressToDescriptor

;===============================================================================
;                               E x p o r t s
;===============================================================================

; Make the enum pre-multiply the values by 4

SEG_GET_ACCESS_RIGHTS equ 0
SEG_GET_EXT_ACCESS_RIGHTS equ 4
SEG_GET_BASE_ADDR equ 8
SEG_GET_LIMIT equ 12

SEG_SET_ACCESS_RIGHTS equ 16
SEG_SET_BASE_ADDR equ 20
SEG_SET_LIMIT equ 24

PROTO Segment_Util

;===============================================================================
                                section .text
;===============================================================================

;-------------------------------------------------------------------------------
; Get/set information about a local descriptor.
; NOTE: This procedure only works with the local descriptor table.
;
; Available to kernel.
;
PROC Segment_Util, U8 func, U16 seg, U32 operand
LOCALS NONE
        movzx   ebx,U16 [argv(.seg)]
        and     bx,0FFF8h
        add     ebx,aqwLocalDescriptorTable

        mov     eax,[argv(.func)]
        add     eax,.brtab
        jmp     eax
.brtab:
        DD .SEG_GET_ACCESS_RIGHTS
        DD .SEG_GET_EXT_ACCESS_RIGHTS
        DD .SEG_GET_BASE_ADDR
        DD .SEG_GET_LIMIT
        DD .SEG_SET_ACCESS_RIGHTS
        DD .SEG_SET_BASE_ADDR
        DD .SEG_SET_LIMIT

.SEG_GET_ACCESS_RIGHTS:
        movzx   eax, U8 [ebx+5]
        ret

.SEG_GET_EXT_ACCESS_RIGHTS:
        ; ...
        ret

.SEG_GET_BASE_ADDR:
        INVOKE  IaGetBaseAddress, ebx
        ret

.SEG_GET_LIMIT:
        lsl     eax, U16 [argv(.seg)]
        ret

.SEG_SET_ACCESS_RIGHTS:
        mov     al,[argv(.operand)]
        mov     [ebx+5],al
        ret

.SEG_SET_BASE_ADDR:
        INVOKE  IaAppendAddressToDescriptor, ebx, U32 [argv(.operand)]
        ret

.SEG_SET_LIMIT:
        INVOKE  IaAppendLimitToDescriptor, ebx, U16 [argv(.operand)]
        ret
ENDPROC




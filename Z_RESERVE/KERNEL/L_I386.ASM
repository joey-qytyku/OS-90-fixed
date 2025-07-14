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

	.386p
	.model  flat, stdcall

;---------------------------------------------------------------------------
;                               E Q U A T E S
;---------------------------------------------------------------------------

LDT_SIZE EQU 512

GDT_NULL        EQU     0
GDT_KCODE       EQU     1
GDT_KDATA       EQU     2
GDT_TSSD        EQU     3
GDT_LDT         EQU     4
GDT_SWCS        EQU     5
GDT_SWDS        EQU     6
GDT_ENTRIES     EQU     7

; The data/stack segment enables the BIG bit
; so that Plug-and-play BIOS recognizes it as
; a 32-bit stack

TYPE_DATA       EQU     0x12
TYPE_CODE       EQU     0x1B

TYPE_LDT        EQU     0x2
TYPE_TSS        EQU     0x9

RING0 EQU 0
RING3 EQU 3
PRES  EQU 1

GRAN  EQU 80h
SEG32 EQU 40h

Access macro present, ring, type
exitm <((present shl 7) or (ring shl 6) or type)>
endm

; Base address is not relocatable, so it must be manually
; written with code.
Desc macro limit, acc, extacc
	DW      (limit) and 0FFFFh
	DW      0
	DW      0
	DB      (acc)
	DB      (extacc) or ((limit) shr 16)
	DB      0
endm

IRQ_BASE        EQU     0A0h
ICW1            EQU     1 shl 4
LEVEL_TRIGGER   EQU     1 shl 3
ICW1_ICW4       EQU     1
ICW4_8086       EQU     1
ICW4_SLAVE      EQU     1 shl 3


externdef       IDT:abs,
externdef       IDT_INFO:fword
extern          GetIrqMask:proc
extern          SetIrqMask:proc
public          TSS
public          LDT

FOR i, <0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19>
	externdef EXC_&i:abs
endm


_BSS    segment align(16) 'BSS'
LDT     DD      LDT_SIZE * 8 DUP(?)
TSS     DD      104+8192 DUP(?)
_BSS    ends

	align 16
GDT     label ptr
	DD      0,0
	Desc    0FFFFFh,        Access(1,0,TYPE_CODE), <SEG32 or GRAN>
	Desc    0FFFFFh,        Access(1,0,TYPE_DATA), <SEG32 or GRAN>
	Desc    103,            Access(1,0,TYPE_TSS),  <0>
	Desc    LDT_SIZE*8-1,   Access(1,0,TYPE_LDT),  <0>
	Desc    512,            Access(1,0,TYPE_CODE), <0>
	Desc    512,            Access(1,0,TYPE_DATA), <0>
EGDT    label ptr

GdtInfo label fword
	dw      EGDT-GDT-1
	dd      GDT


_BSS    ends

_TEXT   segment align=4096 'CODE'

ConfPIC macro
	;
	; Reconfiguring the PICs will destroy the mask registers.
	;
	call    GetIrqMask
	push    eax
	; Different bits tell OCWs and ICWs appart in CMD port
	; Industry standard architecture uses edge triggered interrupts
	; 8-BYTE interrupt vectors are default (ICW[:2] = 0)
	; which is correct for protected mode.
	jmp     $+2
	;ICW1 to both PIC's
	mov     AL,(ICW1 or ICW1_ICW4) ;;;;
	out     20h,al
	jmp     $+2
	out     0A0h,al
	jmp     $+2

	;Set base interrupt vectors
	mov     al,IRQ_BASE
	out     21h,al
	jmp     $+2
	mov     al,IRQ_BASE+8
	out     0A1h,al
	jmp     $+2

	;ICW3, set cascade
	mov     AL,4        ; For master it is a bit mask
	out     21h,AL
	jmp     $+2
	mov     AL,2        ; For slave it is an INDEX
	out     0A1h,AL
	jmp     $+2

	;Send ICW4
	mov     AL,ICW4_8086
	out     21h,AL
	jmp     $+2
	mov     AL,ICW4_8086 or ICW4_SLAVE ; Assert PIC2 is slave
	out     0A1h,AL
	jmp     $+2

	; Rewrite mask register with mask on stack
	call    SetIrqMask

endm

SetupLDTD_LDTR macro
	;---------------------------
	; Create LDT descriptor
	;---------------------------

	push    offset LDT
	push    offset GDT + 8*GDT_LDT
	call    i386SetDescriptorAddress

	;---------------------------
	; Set LDTR
	;---------------------------
	mov     ax,GDT_LDT shl 3
	lldt    ax

endm

SetupTSSD_IOPB macro
	; What to do?
endm

ZeroBSS macro
	cld
	xor     eax,eax
	mov     ecx,offset BSS_SIZE
	mov     edi,offset END_CODE
	rep     stosb
endm

SetupSegments macro
	lgdt    Fword ptr GdtInfo
	mov     ax,2 shl 3
	mov     ds,ax
	mov     es,ax
	mov     ss,ax
	mov     fs,ax
	mov     gs,ax

	DB      0EAh            ; Far JMP
	DD      offset Cont
	DW      8h
Cont:
endm

externdef ISR_REST:abs, ISR_7:abs, ISR_15:abs

;
; ESI = List of vectors
; EDI = IDT base address to start coping to
; ECX = Number of entries to copy
;
CopyVectors PROC
_L:
	movzx   eax, word ptr [esi]
	mov     [edi],ax ;;;

	movzx   eax, word ptr [esi+2]
	mov     [edi+6],ax

	mov     BYTE PTR [edi+5],8Eh
	mov     [edi+2],cs

	add     esi,4
	add     edi,8
	dec     ecx
	jnz     _L

	ret
CopyVectors ENDP

InsertVectors macro

	; Insert exception vectors
	mov     esi,offset IDT_COPY
	mov     edi,offset IDT
	mov     ecx,19
	call    CopyVectors

	; Insert IRQ vectors

	mov     ecx,16
_L:
	push    offset ISR_REST
	loop    SHORT _L

	mov     dword ptr [esp+7*4],ISR_7
	mov     dword ptr [esp+15*4],ISR_15
	mov     esi,esp
	mov     edi,offset(IDT)+(0A0h*8)
	mov     ecx,16
	call    CopyVectors
DONE1:

	lidt    Fword ptr IDT_INFO
endm


SetupRMCA macro

	mov     esi,offset RMCA_COPY
	mov     edi,offset SWBASE
	mov     ecx, (offset RMCA_COPY_END - offset RMCA_COPY) / 4
	cld
	rep     movsd

endm

RMCA_COPY LABEL PTR
	incbin <switch.bin>
	align   4
RMCA_COPY_END LABEL PTR

IDT_COPY:
DD 0, 1, 2, 3, 4, 5, 6, 7, 8
DD 9, 10, 11, 12, 13, 14, 15
DD 16, 17, 18, 19

_TEXT   ends

_INIT   segment 'CODE'

	externdef       END_CODE:abs
	externdef       BSS_SIZE:abs
	extern          KernelMain:PROC

Begin:
	ZeroBSS
	SetupSegments
	ConfPIC

	SetupLDTD_LDTR
	SetupTSSD_IOPB
	InsertVectors
	SetupRMCA

	mov     esp,100000h+65520
	call    KernelMain

	jmp $

_INIT   ENDS

END

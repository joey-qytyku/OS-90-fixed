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
	include OSK/LL/hmadef.inc

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

ACCESS MACRO present, ring, type
EXITM <((present shl 7) or (ring shl 6) or type)>
ENDM

; Base address is not relocatable, so it must be manually
; written with code.
DESC MACRO limit, acc, extacc
	DW      (limit) and 0FFFFh
	DW      0
	DW      0
	DB      (acc)
	DB      (extacc) or ((limit) shr 16)
	DB      0
ENDM

IRQ_BASE        EQU     0A0h
ICW1            EQU     1 shl 4
LEVEL_TRIGGER   EQU     1 shl 3
ICW1_ICW4       EQU     1
ICW4_8086       EQU     1
ICW4_SLAVE      EQU     1 shl 3

;---------------------------------------------------------------------------
;                               I M P O R T S
;---------------------------------------------------------------------------

	EXTERNDEF IDT:ABS, IDT_INFO:FWORD

	EXTERN GetIrqMask:PROC, SetIrqMask:PROC


FOR i, <0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19>
	EXTERNDEF EXC_&i :ABS
ENDM

;---------------------------------------------------------------------------
;                               E X P O R T S
;---------------------------------------------------------------------------

	PUBLIC i386GetDescriptorAddress
	PUBLIC i386SetDescriptorAddress
	PUBLIC i386SetDescriptorLimit
	PUBLIC TSS, LDT

	.DATA?
LDT     DD      LDT_SIZE * 8 DUP(?)
TSS     DD      104+8192 DUP(?)

	.DATA

	align 16
GDT     LABEL PTR
	DD      0,0
	DESC    0FFFFFh,        ACCESS(1,0,TYPE_CODE), <SEG32 or GRAN>
	DESC    0FFFFFh,        ACCESS(1,0,TYPE_DATA), <SEG32 or GRAN>
	DESC    103,            ACCESS(1,0,TYPE_TSS),  <0>
	DESC    LDT_SIZE*8-1,   ACCESS(1,0,TYPE_LDT),  <0>
	DESC    512,            ACCESS(1,0,TYPE_CODE), <0>
	DESC    512,            ACCESS(1,0,TYPE_DATA), <0>
EGDT    LABEL PTR

GdtInfo LABEL FWORD
	DW      EGDT-GDT-1
	DD      GDT

	.CODE
;---------------------------------------------------------------------------
; Arguments on stdcall stack:
;       PVOID   address
;
i386GetDescriptorAddress:
	push    EBX
	; Descriptors are perfectly valid offsets to the LDT when the DPL
	; and GDT/LDT bits are masked off.

	mov     EBX,[ESP+4]

	; EBX is now the address of our descriptor

	; Let us put the segment descriptor address confusion to rest
	; The first address field is 16-bit.
	; It is equal to TheFullAddress & 0xFFFF. Nuff said.
	;
	; The last two fields are BYTEs. To make the high address word
	; we left shift by 8 the 31..24 field
	; then we OR it with the 23..16
	mov     EAX,[EBX+2]
	movzx   ECX,BYTE PTR [EBX+3]
	movzx   EDX,BYTE PTR [EBX+7]        ; 31..24
	shl     EDX,8
	or      ECX,EDX
	or      EAX,ECX

	;EAX now contains the address, we are done.
	pop     EBX
	ret     4

i386SetDescriptorAddress: ;(VOID *gdt_entry, DWORD address)
	push    ebp
	mov     ebp,esp

	push    ebx

	mov     ebx,[ebp+8]

	movzx   eax, WORD PTR [ebp+12]
	mov     [ebx+2],ax

	movzx   eax,BYTE PTR [ebp+12+2]
	mov     [ebx+4],al

	movzx   eax,BYTE PTR [ebp+12+3]
	mov     [ebx+7],al

	pop     ebx

	pop     ebp
	ret     8


i386SetDescriptorLimit:
	push    ebx

	mov     esi, [esp+4]
	mov     dx, [esp+8]

	mov     WORD PTR [esi],dx
	mov     WORD PTR [esi+6],dx

	mov     ah,BYTE PTR [esi+6]
	shl     ax, 8
	mov     BYTE PTR [esi+2], ah

	pop     ebx
	ret     8

SetIntVector:
	push    ebx

	mov     edx,[esp+12+4]    ; Base address to insert
	movzx   eax,BYTE PTR [esp+8+4] ; Info
	mov     ebx,[esp+4+4]     ; Index of descriptor

	; EBX is not the address yet, only index. Calculate.
	lea     ebx,[ebx*8+IDT]

	mov     BYTE PTR [ebx+5],al
	mov     BYTE PTR [ebx+4],0
	mov     [ebx+2],cs
	mov     [ebx+0],dx
	shr     edx,16
	mov     [ebx+6],dx

	pop     ebx
	ret     12

ConfPIC MACRO

	;
	; Reconfiguring the PICs will destroy the mask registers.
	;
	call    GetIrqMask
	push    eax
	; Different bits tell OCWs and ICWs appart in CMD port
	; Industry standard architecture uses edge triggered interrupts
	; 8-BYTE interrupt vectors are default (ICW[:2] = 0)
	; which is correct for protected mode.

	;ICW1 to both PIC's
	mov     AL,(ICW1 or ICW1_ICW4) ;;;;
	out     20h,al
	out     0A0h,al

	;Set base interrupt vectors
	mov     al,IRQ_BASE
	out     21h,al
	mov     al,IRQ_BASE+8
	out     0A1h,al

	;ICW3, set cascade
	mov     AL,4        ; For master it is a bit mask
	out     21h,AL
	mov     AL,2        ; For slave it is an INDEX
	out     0A1h,AL

	;Send ICW4
	mov     AL,ICW4_8086
	out     21h,AL
	mov     AL,ICW4_8086 or ICW4_SLAVE ; Assert PIC2 is slave
	out     0A1h,AL

	; Rewrite mask register with mask on stack
	call    SetIrqMask

ENDM

SetupLDTD_LDTR MACRO
	;---------------------------
	; Create LDT descriptor
	;---------------------------

	push    OFFSET LDT
	push    OFFSET GDT + 8*GDT_LDT
	call    i386SetDescriptorAddress

	;---------------------------
	; Set LDTR
	;---------------------------
	mov     ax,GDT_LDT shl 3
	lldt    ax

ENDM

SetupTSSD_IOPB MACRO
	mov     WORD PTR [TSS+100],104

	;----------------------------------
	; Insert address to TSS descriptor
	;----------------------------------
	push    OFFSET TSS
	push    OFFSET GDT + 8*GDT_TSSD
	call    i386SetDescriptorAddress

	mov     ax,GDT_TSSD shl 3
	ltr     ax
ENDM

ZeroBSS MACRO
	cld
	xor     eax,eax
	mov     ecx,OFFSET BSS_SIZE
	mov     edi,OFFSET END_CODE
	rep     stosb
ENDM

SetupSegments MACRO
	lgdt    FWORD PTR GdtInfo
	mov     ax,2 shl 3
	mov     ds,ax
	mov     es,ax
	mov     ss,ax
	mov     fs,ax
	mov     gs,ax

	DB      0EAh            ; Far JMP
	DD      OFFSET Cont
	DW      8h
Cont:
ENDM

EXTERNDEF ISR_REST:ABS, ISR_7:ABS, ISR_15:ABS

;
; ESI = List of vectors
; EDI = IDT base address to start coping to
; ECX = Number of entries to copy
;
CopyVectors PROC
_L:
	movzx   eax, WORD PTR [esi]
	mov     [edi],ax ;;;

	movzx   eax, WORD PTR [esi+2]
	mov     [edi+6],ax

	mov     BYTE PTR [edi+5],8Eh
	mov     [edi+2],cs

	add     esi,4
	add     edi,8
	dec     ecx
	jnz     _L

	ret
CopyVectors ENDP

InsertVectors MACRO

	; Insert exception vectors
	mov     esi,OFFSET IDT_COPY
	mov     edi,OFFSET IDT
	mov     ecx,19
	call    CopyVectors

	; Insert IRQ vectors

	mov     ecx,16
_L:
	push    OFFSET ISR_REST
	loop    SHORT _L

	mov     DWORD PTR [esp+7*4],ISR_7
	mov     DWORD PTR [esp+15*4],ISR_15
	mov     esi,esp
	mov     edi,OFFSET(IDT)+(0A0h*8)
	mov     ecx,16
	call    CopyVectors
DONE1:

	lidt    FWORD PTR IDT_INFO
ENDM


SetupRMCA MACRO

	mov     esi,OFFSET RMCA_COPY
	mov     edi,OFFSET SWBASE
	mov     ecx, (OFFSET RMCA_COPY_END - OFFSET RMCA_COPY) / 4
	cld
	rep     movsd

ENDM

RMCA_COPY LABEL PTR
	incbin <BLOBS/switch.bin>
	align   4
RMCA_COPY_END LABEL PTR

IDT_COPY:
DD 0, 1, 2, 3, 4, 5, 6, 7, 8
DD 9, 10, 11, 12, 13, 14, 15
DD 16, 17, 18, 19

_INIT   SEGMENT

	EXTERNDEF       END_CODE:ABS
	EXTERNDEF       BSS_SIZE:ABS
	EXTERN          KernelMain:PROC

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

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

	.386
	.model flat, stdcall

	public  memcpy

	.CODE
;
; Maybe use a table of copy operation, perhaps loops in there too?
; That would avoid branches and permit some unrolling.
;
; Basically, we would start at something like EndOfTable - size
;
; I can even use an and operation of some sort to make out of bounds
; copies automatically jump to the loop code.
;

;
; This in an optimized memcpy using stdcall conventions.
; It is designed to be optimal on non-Pentium Pro processors and attempts
; to convert large block transfers to 4-byte copies.

; The i486 takes 7 clocks for MOVS instructions regardless of the size.
; if implemented RISC-style, it would be
; INC EBX + 2xMOV reg<=>mem
;
; On the i486 this is actually less than 7. INC is one clock for register.
; A memory MOV is up to 1 clock cycle assuming cache hits.
; This makes it 3 clocks rather than 7.
;
; Note that only one register needs to be incremented because it is the offset.
;
; A taken branch on the i486 is 3, so now we are at 6.
;
; According to the manual, when (E)CX is greater than 1 the, each the time of
; REP MOVS is 3*ECX. This may seem better than the RISC version, but the CX=1
; case uses 13 clocks and the CX=0 case uses 5 clocks. This means 18 clocks
; are used. This is 12 more than needed since the transfers are 6.
;
; On the i386, REP MOVS uses 4 clocks per iteration but has only a 5 clock startup.
; Memory move operations are 2 clocks.
;
; Algebra can be used to figure out the threshold for when it is prefered
; to use the RISC or CISC version. For a kernel this is not a good idea.
;----------------------------------------------------------------------------------
; Next topic is memset. As stated before, a memory access is 1 clock minimum
; on the i486 and 2 on the i386.
;
; REP STOS is 7+4(E)CX on the i486. This cannot be beaten with a 3-clock branch
; and a 2-clock move.
;
; The takeaway is:
; - Use an unroll copy operation table for small copies (<=32)
; - Anything larger, use MOVSD for each 4-byte quantity and MOVSB for the rest
; - String operations are unbeatable for large transfers.

; NOTE: I want small copies to be very fast. This means that 4-byte folding
; needs to happen for those too.
; Also, if it is known that something is DWORD-granular, that should be used instead.
; Like the memcpy4 in SDL.

	align  64
memcpy:
	push    esi
	push    edi
	cld

	mov      edi,[esp+8+(8)]
	mov      esi,[esp+8+(12)]
	mov      edx,[esp+8+(16)]

	cmp      edx,16
	jb       small_block

	mov      ecx,edx
	shr      ecx,2
	rep      movsd

	; Copy remaining bytes
	mov      ecx,edx
	and      ecx,11b
	rep      movsb
	jmp      done

small_block:
	mov      ecx,edx
	rep      movsb
done:
	pop      edi
	pop      esi
	ret      12

	align  64
memcmp:

	END

;
; Same as memcpy but regions can overlap.
;
; If the copy is forward, perform backward copy starting from end of region.
; If copy is backward, copy from end of source to begining of destination.
;
M_memmove:

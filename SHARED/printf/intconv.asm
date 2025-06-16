; Copright (C) 2025 Joey Qytyku, All Rights Reserved

; This is a optimized integer conversion function.
; It is supposed to be really fast on 386/486/586.
; Not sure if it is any good on a modern CPU.

; Can 64-bit be done like this too using the same code?
; It can be actually.
; It requires a division to get a 32-bit result, and break up the digits
;
; Because 64-bit requires 19 characters to represent, we can divide it by
; 10^(19/2) or 10^9
;
; But we should really make that 10^10 to make it one operation.
;
; Dividing will basically "split" the number between quotient and remainder,
; which are the subjects of TWO conversion operations.
; This requires zero libgcc calls.
;
; It has one critical problem, and that is the lack of alignment, which
; makes the 32-bit writes 4 times slower 75% of the time.
; Probably unavoidable though, unless I make a specialized procedure only
; for 64-bit conversions.
;

global _asm_v
extern _printf, _putchar

section .data

msg db "{%u:%u}",10,0

; I should set up architectural alignments around here.

	align   32
table:
%assign i 0
%rep 1000
	db '0'
	db '0'+((i/100   ))
	db '0'+((i / 10) %% 10)
	db '0'+(i %% 10 )
	%assign i i+1
%endrep

section .text

	align   32
_asm_v:
	push	ebp
	push	edi
	push	ebx

	mov	ebx,1000
    	mov	edi,[esp+12 +8]   ; EDI = buffer, we need it for later
    	mov	eax,[esp+12 +4]   ; EAX = value

	; This number, when divided by 1000, results in Q=999, R=999
	; which is in range of our lookup table.
	; This makes conversions under 1 million much faster
	; and avoids more than one division.
	cmp	eax,999_999
	ja	.full_conversion

.under_1m:
	; Split the value between DX:AX for division
	mov	edx,eax
	shr	edx,16
	movzx	eax,ax

	div	bx

	; Only three of the characters I get are actually valid.
	; A 32-bit read is used for speed.
	mov	ecx,[edx*4+table]
	mov	[edi-3],ecx

	mov	ecx,[eax*4+table]
	mov	dword [edi-6],ecx

	mov	ecx,6
	sub	edi,6

	jmp	.done

	align	4
.full_conversion:

	; When we know that the quotient is representable by a smaller type
	; a narrower divide can be used to save clocks.
	; DO NOT OMIT XOR. There will almost always be a remainder.

	; For now, 32-bit division is unavoidable.

	; Immediate values have a penalty on i486, also this makes encoding
	; take less bytes.
	mov	ebp,table

	xor	edx,edx
	div	ebx
	mov	ecx,[edx*4 + ebp]
	mov	[edi-3],ecx

	; We fetch 4 bytes and one of them is invalid, so we must overwrite.

	; EAX actually contains a value that if divided by 1000, gives
	; a valid 16-bit integer.
	; Problem is, our quotient is now in EAX and not DX:AX.
	; So do that.

	mov	edx,eax
	shr	edx,16
	movzx	eax,ax

	div	bx

	; BTW the original input is byte-aligned so the offset actually is
	; at a 32-bit boundary.

	; AX=result, DX=remainder
	mov	ecx,[edx*4 + ebp]
	; This time, write CL back to the buffer, then shift.
	; Avoids tail byte overhead

	; FLAG: possible error or later overwite
	mov	[edi-6],ecx

	; We don't need DX anymore. It will be zeroed out by this
	; and the value in EAX is representable in 16 bits now,
	; including the remainder which we calculate the final char with.
	xor	edx,edx
	div	bx

	mov	ecx,[edx*4 + ebp]
	mov	[edi-9],ecx

	add	eax,'0'
	mov	byte[edi-9],al

	mov	ecx,3
	sub	edi,9

.done:
	mov	eax,'0'
	repe	scasb

	lea	eax,[edi-1]

	pop	ebx
	pop	edi
	pop	ebp
	ret

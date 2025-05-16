%ifdef TESTING
%define NAME _my_memcmp
%endif


section .text
global _my_memcmp
	align	CACHE_LINE_BOUNDARY
_my_memcmp :
	push	esi
	push	edi

	mov	esi,[esp+4  +4+4]
	mov	edi,[esp+8  +4+4]
	mov	ecx,[esp+12 +4+4]

	mov	edx,ecx

	; Set EDX to the number of extraneous bytes that cannot be DWORD'ed
	and	edx,3

	; Clear EAX because we will use setCC instructions.
	xor	eax,eax

	; Compute number of double words with MOD 4
	shr	ecx,2

	repe	cmpsd

	; We have to be very careful here and make sure the code does not fail
	; on rare edge cases.
	;
	; Basically if the DWORDs we compared were NOT equal, then that means
	; the DWORDs must be compared again by the byte to find which one is
	; different.
	;
	; If they ARE equal, then we should scan whatever bytes are left.
	;
	; The shared path is that both situations share is setting the result
	; based on a byte comparison.

	jne	.prepare_check_again

	mov	ecx,edx
	jmp	.cont

.prepare_check_again:
	sub	edi,4
	sub	esi,4
	mov	ecx,4
.cont:
	repe	cmpsb

	; movzx	edx,byte[edi-1]
	; movzx	eax,byte[esi-1]
	; push	edx
	; push	eax
	; push	msg
	; call	_printf
	; jmp $

	movzx	eax,byte[esi-1]
	movzx	edx,byte[edi-1]
	sub	eax,edx

	pop	edi
	pop	esi
	ret

; Maybe I should use JECXZ?

; msg: DB "Value = %x:%x",10,0

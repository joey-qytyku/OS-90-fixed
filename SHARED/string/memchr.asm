%ifdef TESTING
%define NAME _my_memchr
%endif

global NAME

section .text

	align	32
NAME:
	push	edi
	mov	edi,[esp+8]
	mov	eax,[esp+12]
	mov	ecx,[esp+16]

	repne	scasb
	lea	edi,[edi-1]
	lea	eax,[0]
	setz	al
	imul	edi
	pop	edi
	ret

;
; The IMUL will take 9 clocks in all cases because the multiplier can only be
; 0 or 1.
; The clocks used are: max(ceiling(log{2}(m)), 3) + 6
;

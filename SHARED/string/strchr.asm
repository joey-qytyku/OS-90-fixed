%ifdef TESTING
%define NAME my_strchr
%else
%define NAME strchr
%endif


section .text
global _my_strchr

	align	32
_my_strchr :
	push	esi
	mov	esi,[esp+8]
	mov	edx,[esp+12]
.L:
	lodsb
	cmp	al,0
	jz	.not_found
	cmp	al,dl
	jnz	.L

	lea	eax,[esi-1]
	pop	esi
	ret

.not_found:
	xor	eax,eax
	pop	esi
	ret

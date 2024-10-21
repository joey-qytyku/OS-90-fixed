
global  M_fast_memcpy
global  M_fast_memset
global  M_fast_memzero
global  M_fast_memset2

section .text

	align   32
M_fast_memcpy:
	cld
	mov     eax,ecx
	shr     ecx,2
	rep     movsd
	mov     ecx,eax
	and     ecx,3
	rep     movsb
	ret

	align   32
M_fast_memset:
	cld
	imul    eax,eax,1010101h

	mov     ebx,ecx
	shr     ecx,2
	rep     stosd

	mov     ecx,ebx
	and     ecx,3
	rep     stosb
	ret

	align   32
M_fast_memzero:
	cld
	xor     eax,eax

	mov     ebx,ecx
	shr     ecx,2
	rep     stosd

	mov     ecx,ebx
	and     ecx,3
	rep     stosb
	ret

M_fast_memset2:
	cld
	imul    eax,eax,00010001h

	mov     ebx,ecx
	shr     ecx,1
	rep     stosd

	and     ebx,1
	jz      finish
	mov     dword [edi],0
	finish:
	ret



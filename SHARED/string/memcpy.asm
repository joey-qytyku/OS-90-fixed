global memcpy

	align	32
memcpy:
	push	esi
	push	edi

	mov	edi,[esp+12]
	mov	esi,[esp+8]
	mov	ecx,[esp+4]

	mov	ecx,edx
	shr	ecx,2
	and	edx,11b

	rep	movsd

	mov	ecx,edx
	rep	movsb

	pop	edi
	pop	esi
	ret

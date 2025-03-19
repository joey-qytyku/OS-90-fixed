	global strchr

	align	32
strchr:
	push	edi
	mov	edi,[esp+12]
	mov	eax,[esp+8]
	repe	scasb
	lea	eax,[edi-4]
	pop	edi
	ret

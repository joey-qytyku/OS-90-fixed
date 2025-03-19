__attribute__((cdecl, regparm(0)))
__SIZE_TYPE__ strlen(const char *__s)
{
	__SIZE_TYPE__ s;
	__asm__ volatile (
		"mov %1,%%edi\n\t"
		"mov    $0xFFFFFFFF,%%ecx\n\t"
		"xor     %%eax,%%eax\n\t"
		"repnz   scasb\n\t"

		"leal    1(%%ecx),%0\n\t"
		"not     %0\n\t"
		:"=rm"(s)
		:"edi"(__s)
		:"memory","edi","ecx","eax"
	);
	return s;
}

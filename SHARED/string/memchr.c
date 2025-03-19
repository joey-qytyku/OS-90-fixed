__attribute__((cdecl, regparm(0)))
void *_memchr(const void* ptr, int ch, __SIZE_TYPE__ count)
{
	asm (
		"push %edi\n\t"
		"movl 8(%esp),%edi\n\t"
		"movl 12(%esp),%eax\n\t"
		"movl 16(%esp),%ecx\n\t"
		"repne scasb\n\t"
		"jnz   0f\n\t"
		"lea -1(%edi),%eax\n\t"
		"pop %edi\n\t"
		"ret\n\t"
		"0:xorl %eax,%eax\n\t"
		"pop %edi\n\t"
		"ret\n\t"
    );
}
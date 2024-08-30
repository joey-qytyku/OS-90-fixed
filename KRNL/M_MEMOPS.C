#include "M_MEMOPS.H"

void M_fast_memcpy(void * __restrict to, void * __restrict from, size_t size)
{
	_asm {
		cld
		mov eax,ecx
		shr ecx,2
		rep movsd
		mov ecx,eax
		and ecx,3
		rep movsb
	};
}

// Unrolling stosb may not be fast. Maybe find a way to use stosw there

//
// TESTED WORKING
//
void M_fast_memset(void *buff, int val, size_t size)
{
	_asm {
		cld
		imul eax,eax,1010101h

		mov ebx,ecx
		shr ecx,2
		rep stosd

		mov ecx,ebx
		and ecx,3
		rep stosb
	};
}

void M_fast_memzero(void *buff, size_t size)
{
	// Unroll using table? There will be 0,1,2,3 iterations. No more.
	_asm {
		cld
		xor eax,eax

		mov ebx,ecx
		shr ecx,2
		rep stosd

		mov ecx,ebx
		and ecx,3
		rep stosb
	};
}

// Why use a stosw? I dont need the decrement, do I?
// It comes after, I think.
void M_fast_memset2(void *buff, int val, size_t words)
{
	_asm {
		cld
		imul eax,eax,00010001h

		mov ebx,ecx
		shr ecx,1
		rep stosd

		and ebx,1
		jz finish
		mov dword ptr [edi],0
		finish:
	};
}

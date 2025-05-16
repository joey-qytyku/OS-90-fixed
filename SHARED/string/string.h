#ifndef SHARED_STRING_H
#define SHARED_STRING_H

#define memcpy		__builtin_memcpy
#define memmove		__builtin_memmove
#define strcpy		__builtin_strcpy
#define strncpy		__builtin_strncpy
#define strcat		__builtin_strcat
#define strncat		__builtin_strncat
#define memcmp		__builtin_memcmp
#define strcmp		__builtin_strcmp
// #define strcoll		__builtin_strcoll
#define strncmp		__builtin_strncmp
#define memchr		__builtin_memchr
#define strchr		__builtin_strchr
#define strcspn		__builtin_strcspn
#define strpbrk		__builtin_strpbrk
#define strrchr		__builtin_strrchr
#define strspn		__builtin_strspn
#define strstr		__builtin_strstr
// #define strtok		__builtin_strtok
#define memset		__builtin_memset
// #define strerror	__builtin_strerror
// #define strxfrm		__builtin_strxfrm

// GCC goes not inline strlen, even on new versions. We do it here manually.
// The size is not that big, only 12 bytes.
//
static inline __SIZE_TYPE__ strlen(const char *__s)
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

#endif /*SHARED_STRING_H*/

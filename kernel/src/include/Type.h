#ifndef TYPE_H
#define TYPE_H

#include <stdalign.h>

///////////////////////////////////////////////
// K e r n e l   A P I   E x i t   C o d e s //
///////////////////////////////////////////////

enum {
OS_OK,
OS_ERROR_GENERIC,
OS_DOS_ERROR,
OS_INVALID_FUNCTION,
OS_INVALID_PARAMS,

OS_ACCESS_DENIED,
OS_FILE_NOT_FOUND,
OS_INVALID_HANDLE,
OS_IS_DIRECTORY_NOT_FILE,
OS_IS_FILE_NOT_DIRECTORY,
OS_TOO_MANY_OPEN,
OS_NO_SPACE_ON_DISK,
OS_FEATURE_NOT_SUPPORTED,
OS_OUT_OF_MEMORY
};

/////////////////////////////////////////////////////////////////////////////
// I n l i n e   A s s e m b l y   a n d   A s s e m b l y   L i n k a g e //
/////////////////////////////////////////////////////////////////////////////

#define ASNL "\n\t"
#define ASM_LINK __attribute__(( regparm(0) ))

////////////////////////////////////////////////////
// F u n c t i o n   R e l a t e d   M a c r o s  //
////////////////////////////////////////////////////

#define KERNEL  __attribute__(( regparm(0), cdecl, noinline ))
#define KERNEL_ASYNC KERNEL
#define BYTESWAP(value) ((value & 0xFF) << 24) | ((value & 0xFF00) << 8) |\
((value & 0xFF0000)>>8) | ((value & 0xFF000000) >> 24)

#define UNUSED_PARM(x) (VOID)x

///////////////////////////////////////////
// M i s c e l a n e o u s   M a c r o s //
///////////////////////////////////////////

// Volatile variables can change at any time without the
// compiler being aware. This applies to ISRs and drivers
// because they are unpredictable and also assembly code that
// modifies a C variable
#define INTVAR volatile /* Used by interrupt handler  */
#define MCHUNX volatile /* May change unexpectedly */

#define ALIGN(x) __attribute__( (aligned(x)) )

#define FENCE __asm__ volatile ("":::"memory")

#define offsetof(st, m) ((DWORD)&(((st *)0)->m))

/////////////////////////////////////
// T y p e   D e f i n i t i o n s //
/////////////////////////////////////

#define VOID void

#define BYTE unsigned char
#define WORD unsigned short
#define DWORD unsigned long
#define QWORD unsigned long long

#define SBYTE char
#define SWORD short
#define SDWORD long
#define SQWORD long long

#define PVOID VOID*
#define PWORD WORD*
#define PDWORD DWORD*
#define PBYTE BYTE*

#define BOOL _Bool
#define STATUS DWORD
#define HANDLE SDWORD

#define PID WORD
#define VINT DWORD

typedef const char*const IMUSTR;

// Pointer to IMUSTR may change, but the IMUSTR itself must not.
typedef IMUSTR *PIMUSTR;

#define NULL ((PVOID)0L)

#define tstruct   typedef struct
#define tpkstruct typedef struct __attribute__((packed))

#define likely(x)   __builtin_expect((x),1)
#define unlikely(x) __builtin_expect((x),0)

/////////////////////////////////////////////////////////////////////
// A d d r e s s + O f f s e t   A d d r e s s i n g   M a c r o s //
/////////////////////////////////////////////////////////////////////

#define BYTE_PTR(var, off) *(PBYTE) (var+off)
#define WORD_PTR(var, off) *(PWORD) (var+off)
#define DWORD_PTR(var,off) *(PDWORD)(var+off)

#define PTR2INT(ptr) ((DWORD)(ptr))
#define INT2PTR(Int, ptrtype) ((ptrtype)(Int))

#define CAST(val, type) ((type)(val))

#define FLAG_PARAM_ON(flags, mask) ((flags & mask)!=0)


/////////////////////////////
// G C C   B u i l t i n s //
/////////////////////////////

static inline VOID *C_memcpy(VOID *d, VOID *s, DWORD c)
{
	return __builtin_memcpy(d, s, c);
}

static inline VOID *C_memmove(VOID *d, VOID *s, DWORD c)
{
	return __builtin_memmove(d, s, c);
}

static inline int C_strcmp(char *s1, char *s2)
{
	return __builtin_strcmp(s1, s2);
}

static inline VOID C_memset(PVOID a, DWORD val, DWORD count)
{
    __builtin_memset(a,val,count);
}

#endif /* TYPE_H */

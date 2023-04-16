#ifndef TYPE_H
#define TYPE_H

#define VOID void

// IMUSTR is a constant pointer to a constant string. The compiler
// will generate only the string when optimizing and no pointer.
//
// PIMMUTABLE_STRING is a double pointer type. It makes strings simpler because
// it uses the & operator to get the address, which is always the actual data
// of the IMMUTABLE_STRING and constistent with other types.
//
// There is no reason to modify a C string. An array/buffer should be used
// to handle string processing rather than a C string.
//
// Examples:
// IMUSTR str = "Hello, world!\n";
// PIMUSTR pstr = &str;

// printf("%s", *pstr);

typedef const char*const IMUSTR;

//
// Pointer to IMUSTR may change, but the IMUSTR itself must not.
//
typedef IMUSTR *PIMUSTR;

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

#define offsetof(st, m) \
    ((size_t)&(((st *)0)->m))

#define ZERO_STRUCT {0}

#define IN   /* This argument is a pointer to an input value*/
#define OUT  /* output */

#define BIT_IS_SET(num,bit) ((num & (1<<bit))>0)
#define NULL ((PVOID)0L)

#define ASNL "\n\t"

#define ASM_LINK __attribute__(( regparm(0) ))
#define APICALL ASM_LINK

// A function with this attribute is reentrant and can execute successfully
// even if it is interrupted. ISRs can ONLY call reentrant functions.
#define ASYNC_APICALL APICALL

// The BSWAP instruction is only supported by i486 and above
// but this is only a macro. I figured this out myself :)
#define BYTESWAP(value) ((value & 0xFF) << 24) | ((value & 0xFF00) << 8) | ((value & 0xFF0000)>>8) | ((value & 0xFF000000) >> 24)


// Volatile variables can change at any time without the
// compiler being aware. This applies to ISRs and drivers
// because they are unpredictable and also assembly code that
// modifies a C variable
#define INTVAR volatile /* Used by interrupt handler  */
#define MCHUNX volatile /* May change unexpectedly */

#define __PACKED   __attribute__( (packed) )
#define __ALIGN(x) __attribute__( (aligned(x)) )

typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned long  DWORD;

typedef char	SBYTE;
typedef short	SWORD;
typedef long	SDWORD;
typedef	long long SQWORD;
typedef	unsigned long long QWORD;

typedef VOID*	  PVOID;
typedef WORD*     PWORD;
typedef BYTE*     PBYTE;
typedef DWORD*    PDWORD;
typedef QWORD*	  PQWORD;

typedef _Bool   BOOL;
typedef DWORD  STATUS;
typedef SDWORD HANDLE;

typedef WORD PID;

// Abstract type representing an interrupt vector
// or virtual interrupt
typedef DWORD VINT;


// Builtin functions use inline x86 string operations
// making them way faster that doing it in C.
// string operand size can also be deduced by the compiler

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

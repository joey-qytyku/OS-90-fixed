////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                     Copyright (C) 2023, Joey Qytyku                        //
//                                                                            //
// This file is part of OS/90 and is published under the GNU General Public   //
// License version 2. A copy of this license should be included with the      //
// source code and can be found at <https://www.gnu.org/licenses/>.           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////


#ifndef TYPE_H
#define TYPE_H

#include <stdalign.h>

///////////////////////////////////////////////
// K e r n e l   A P I   E x i t   C o d e s //
///////////////////////////////////////////////

// Are we even going to use all these?

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

#define KERNEL  __attribute__(( regparm(0), cdecl ))
#define KERNEL_ASYNC KERNEL
#define BYTESWAP(value) ((value & 0xFF) << 24) | ((value & 0xFF00) << 8) |\
((value & 0xFF0000)>>8) | ((value & 0xFF000000) >> 24)

#define UNUSED_PARM(x) (VOID)x

// stdarg? Pfffff. Too bloated. We know we are using cdecl and that the callee
// is going to clean up the arguments, so might as well use a pointer.
// On the stack, each item is going to be low-to-high in memory since cdecl
// goes in reverse (it has to for variadic arguments to work reasonably).
//
// So no need for va_start and va_end!

#define GET_VAR_LIST(last_fixed) (&(last_fixed) + 4)

// API_DECL creates a type definition for a function
// so that it can be added to the procedure table

#define API_DECL(rtype, name, ...)\
    typedef rtype KERNEL (*_API_##name)(__VA_ARGS__);\
    rtype KERNEL name(__VA_ARGS__)

///////////////////////////////////////////
// M i s c e l a n e o u s   M a c r o s //
///////////////////////////////////////////

#define ALIGN(x) __attribute__( (aligned(x)) )

#define FENCE __asm__ volatile ("":::"memory")

#define offsetof(st, m) ((U32)&(((st *)0)->m))

/////////////////////////////////////
// T y p e   D e f i n i t i o n s //
/////////////////////////////////////

#define VOID void
#define PVOID void*

#define BOOL _Bool
#define STATUS U32
#define HANDLE I32

#define PID U16
#define VINT U32

typedef __UINT32_TYPE__ U32,*PU32;
typedef __UINT16_TYPE__ U16,*PU16;
typedef __UINT8_TYPE__  U8, *PU8;

typedef __INT32_TYPE__ S32,*PS32;
typedef __INT16_TYPE__ S16,*PS16;
typedef __INT8_TYPE__  S8, *PS8;

typedef __UINT64_TYPE__ U64,*PU64;

/*
Value of a variable of type IMUSTR (referencing by name only) is the address.
sizeof will return the size of the string including characters.
Address of is the same as getting the value.

C has a bizzare way of handling strings and how they relate to arrays.

In C, it is legal to do:
    char *str = "Hello";
But not:
    int *nums = {1,2,3,4};
Why is that? I don't know. There is no real difference except the size of
each element of the not-really-an-array.

In C, you can also do:
    char str[] = "Hello";

Lets go over what they actually generate using GCC.

C:
    char *str = "..."; // static does not affect

ASSEMBLY:
    A pointer is generated that points to the string.
    It can be reassigned. Not suprising, but why cant you do this
    with other types?

    Also the

C:
    const char *str = "...";

ASSEMBLY:
    A pointer is generated, despite the type being
    "a constant pointer to a char"

C:
    char str[] = "Hello";

ASSEMBLY:
    A single label with an array of characters is generated. The value and
    address of the variable is the address of the first element.

Extremely confusing. There are several different ways to make strings with their
own quirks.

We need a solution to separate the idea of a string from the idea of a pointer.
Somehting like the Java String type is not practical since this is C and we
want to be able to easily change strings.
Solution:
    STR is a char[] type. It can be externed. The value and & of the variable is
    address of first element, but & is preffered. STR* is a pointer to one and
    passing &mystr to a function would be the same as passing the value.

    // This would work
    VOID PrintString(STR *thestr)
    {
        printf("%s", &thestr);
    }

    The concept of a pointer and a string are thus separated. Now there can be
    a pointer to a string that is really just the value of the ASM label
    internally. STR presents itself like any other type, but is compatible with
    assembly procedures that takes byte pointers.
*/

// Abolish this and simply use const char*

#define NULL ((PVOID)0UL)

#define tstruct   typedef struct
#define tpkstruct typedef struct __attribute__((packed))

#define likely(x)   __builtin_expect((x),1)
#define unlikely(x) __builtin_expect((x),0)

/////////////////////////////////////////////////////////////////////
// A d d r e s s + O f f s e t   A d d r e s s i n g   M a c r o s //
/////////////////////////////////////////////////////////////////////

#define BYTE_PTR(var, off) *(PU8) ((var)+(off))
#define WORD_PTR(var, off) *(PU16) ((var)+(off))
#define DWORD_PTR(var,off) *(PU32)((var)+(off))

#define PTR2INT(ptr) ((U32)(ptr))
#define INT2PTR(Int, ptrtype) ((ptrtype)(Int))

// Consider removing these two
#define CAST(val, type) ((type)(val))
#define FLAG_PARAM_ON(flags, mask) ((flags & mask)!=0)

/////////////////////////////
// G C C   B u i l t i n s //
/////////////////////////////

static inline VOID *C_memcpy(VOID *d, VOID *s, U32 c)
{
	return __builtin_memcpy(d, s, c);
}

static inline VOID *C_memmove(VOID *d, VOID *s, U32 c)
{
	return __builtin_memmove(d, s, c);
}

static inline int C_strcmp(char *s1, char *s2)
{
	return __builtin_strcmp(s1, s2);
}

static inline VOID C_memset(PVOID a, U32 val, U32 count)
{
    __builtin_memset(a,val,count);
}

#endif /* TYPE_H */

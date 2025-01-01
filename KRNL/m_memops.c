/////////////////////////////////////////////////////////////////////////////
//                     Copyright (C) 2022-2024, Joey Qytyku                //
//                                                                         //
// This file is part of OS/90.                                             //
//                                                                         //
// OS/90 is free software. You may distribute and/or modify it under       //
// the terms of the GNU General Public License as published by the         //
// Free Software Foundation, either version two of the license or a later  //
// version if you chose.                                                   //
//                                                                         //
// A copy of this license should be included with OS/90.                   //
// If not, it can be found at <https://www.gnu.org/licenses/>              //
/////////////////////////////////////////////////////////////////////////////

// These are not be called directly.

__attribute__((aligned(16)))
VOID M_fast_memcpy_(VOID)
{
        __asm__ volatile(
        "push    %eax\n"
        "push    %ebx\n"
        "push    %ecx\n"
        "push    %esi\n"
        "push    %edi\n"

        "cld\n"
        "movl    %ecx,%eax\n"
        "shrl    $2,%ecx\n"
        "rep\n"
        "movsl\n"
        "movl    %eax,%ecx\n"
        "andl    $3,%ecx\n"
        "rep\n"
        "movsb\n"

        "pop     %edi\n"
        "pop     %esi\n"
        "pop     %ecx\n"
        "pop     %ebx\n"
        "pop     %eax\n"
        "ret\n"
        );
}

__attribute__((aligned(16)))
VOID M_fast_memset_(VOID)
{
        __asm__ volatile(
        "push    %eax\n"
        "push    %ebx\n"
        "push    %ecx\n"
        "push    %edi\n"

        "cld\n"
        "imull   $0x01010101,%eax,%eax\n"

        "movl    %ecx,%ebx\n"
        "shrl    $2,%ecx\n"
        "rep stosl\n"

        "movl    %ebx,%ecx\n"
        "andl    $3,%ecx\n"
        "rep stosb\n"

        "pop     %edi\n"
        "pop     %ecx\n"
        "pop     %ebx\n"
        "pop     %eax\n"
        "ret\n"
        );
}

__attribute__((aligned(16)))
VOID M_fast_memzero_(VOID)
{
        __asm__ volatile(
        "push    %eax\n"
        "push    %ebx\n"
        "push    %ecx\n"

        "cld\n"
        "xorl    %eax,%eax\n"

        "movl    %ecx,%ebx\n"
        "shrl    $2,%ecx\n"
        "rep stosl\n"

        "movl    %ebx,%ecx\n"
        "andl    $3,%ecx\n"
        "rep stosb\n"

        "push    %ecx\n"
        "push    %ebx\n"
        "push    %eax\n"
        "ret\n"
        );
}

        // .align 16
__attribute__((aligned(16)))
VOID M_fast_memset2_(VOID)
{
        __asm__ volatile (
        "push    %eax\n"
        "push    %ebx\n"
        "push    %ecx\n"
        "push    %edi\n"

        "cld\n"
        "imull   $0x00010001,%eax,%eax\n"

        "movl    %ecx,%ebx\n"
        "shrl    %ecx\n"
        "rep\n"
        "stosl\n"

        "andl    $1,%ebx\n"
        "jz      finish\n"
        "movl    $0,(%edi)\n"
"finish:\n"
        "pop     %edi\n"
        "pop     %ecx\n"
        "pop     %ebx\n"
        "pop     %eax\n"
        "ret\n"
        );
}

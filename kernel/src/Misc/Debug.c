///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                     Copyright (C) 2023, Joey Qytyku                       //
//                                                                           //
// This file is part of OS/90 and is published under the GNU General Public  //
// License version 2. A copy of this license should be included with the     //
// source code and can be found at <https://www.gnu.org/licenses/>.          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


#include <Scheduler/V86M.h>

#include <Platform/IO.h>

#include <stdarg.h>
#include <Debug.h>
#include <Type.h>

#include <Misc/String.h>

#define MAX_STR_LENGTH_OF_UINT32 10

// GCC builtin always refers to the glibc function for some reason
// so I have to implement it manually
U32 KERNEL StrLen(PIMUSTR ps)
{
    U32 i = 0;
    while ((*ps)[i] !=0)
        i++;
    return i;
}

//
// The following functions are probably slow, but there does not
// seem to be a perfect way of doing it, besides this one I found?
// https://www.quora.com/What-are-the-most-obscure-useless-x86-assembly-instructions?
//

// VOID KERNEL Hex32ToString(U32 value,  PU8 obuffer)
// {
// }

// NOW IN ASSEMBLY!
//
// VOID KERNEL Uint32ToString(U32 value, PU8 obuffer)
// {
//     U32 digit, digit_divisor;
//     U32 buff_off = MAX_STR_LENGTH_OF_UINT32 - 2;
//     U32 i;

//     // Clear buffer by setting all chars to ascii NUL
//     // so that they are not printed
//     C_memset(obuffer, '\0', MAX_STR_LENGTH_OF_UINT32);

//     // The following loops through each  *digit
//     // an then copies them to the buffer in reverse order
//     // the integer digit is then converted to a character

//     // It looks complicated, but dividing by 1, 10, 100, etc.
//     // is like bitwise shifting but for decimal digits, not binary.
//     // Modulus is like an AND operation, getting the remainder
//     // or "offset" in ComSci terms. Together, it is a sort of shift/and loop

//     for (i=0, digit_divisor=1; i<MAX_STR_LENGTH_OF_UINT32; i++)
//     {
//         digit = (value / digit_divisor) % 10;
//         obuffer[buff_off] = '0' + digit;

//         // ENDING STATEMENTS
//         digit_divisor *= 10;
//         buff_off--;
//     }
// }

VOID KeWriteAsciiz(OUTPUT_DRIVER od, IMUSTR string)
{
    U32 max = StrLen(&string);

    for (U32 i = 0; i<max; i++)
        od(string[i]);
}

// @x - Hex32
// @i - Int32
// @s - string (@/# irrelevant)
// # for signed
// Example:
//  KeLogf(LptDebug, "Value = @d\n\r", value)
//
//
// It is the output driver's responsibility to handle ascii sequences
// Logf sends the character when it is not a format escape
//
VOID KERNEL KeLogf(OUTPUT_DRIVER od, IMUSTR fmt, ...)
{
    U8 printfmt_buffer[MAX_STR_LENGTH_OF_UINT32 + 1];
    va_list ap;
    BOOL is_signed;

    va_start(ap, fmt);

    for (U16 i=0; fmt[i] != 0; i++)
    {
        if     (fmt[i] == '@') {is_signed = 0;}
        else if(fmt[i] == '#') {is_signed = 1;}
        else               {od(fmt[i]); continue;}

        // The following runs if this is a format character
        i+=2; // Skip the format characters

        switch (fmt[i-1])
        {
        // Print hexadecimal, sign is ignored
        case 'x':
            va_arg(ap, U32);
        break;

        // Print integer, signed or unsigned format
        case 'i':
        break;

        case 's':
            // TODO?
        break;

        case '#':
        case '@': od(fmt[i-1]);
        break;

        default:
            goto FailSilent;
        }
    }

    FailSilent:
    // This function must end with va_end
    va_end(ap);
}

VOID _KernelPutchar(U8 ch)
{
    outb(0xE9, ch);
}

VOID KERNEL FatalError(U32 error_code)
{
}

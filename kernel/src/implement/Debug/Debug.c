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
#include <Debug/Debug.h>
#include <Type.h>

#include <Misc/String.h>

#define MAX_STR_LENGTH_OF_UINT32 10

//
// The following functions are probably slow, but there does not
// seem to be a perfect way of doing it, besides this one I found?
// https://www.quora.com/What-are-the-most-obscure-useless-x86-assembly-instructions?
//

// VOID KERNEL Hex32ToString(U32 value,  PU8 obuffer)
// {
// }

//
// This will zero the entire buffer.
//
//
ARGPTR_WRO(2)
VOID KERNEL Uint32ToString(U32 value, char *obuffer)
{
    U32 digit, digit_divisor;
    U32 buff_off = MAX_STR_LENGTH_OF_UINT32 - 2;
    U32 i;

    // Clear buffer by setting all chars to ascii NUL
    // so that they are not printed
    C_memset(obuffer, '\0', MAX_STR_LENGTH_OF_UINT32);

    // The following loops through each  *digit
    // an then copies them to the buffer in reverse order
    // the integer digit is then converted to a character

    // It looks complicated, but dividing by 1, 10, 100, etc.
    // is like bitwise shifting but for decimal digits, not binary.
    // Modulus is like an AND operation, getting the remainder
    // or "offset" in ComSci terms. Together, it is a sort of shift/and loop

    for (i=0, digit_divisor=1; i<MAX_STR_LENGTH_OF_UINT32; i++)
    {
        digit = (value / digit_divisor) % 10;
        obuffer[buff_off] = '0' + digit;

        // ENDING STATEMENTS
        digit_divisor *= 10;
        buff_off--;
    }
}

VOID KERNEL Putchar(char ch)
{
    outb(0xE9, ch);
}

static OUTPUT_DRIVER od = Putchar;

// strlen is def sus

VOID KERNEL WriteAsciiz(const char *string)
{
    for (U32 i = 0; string[i] != 0; i++)
        od(string[i]);
}

VOID SetOutputDriver(OUTPUT_DRIVER od)
{}

// @x - Hex32
// @i - Int32
// @s - string (@/# irrelevant)
// # for signed
// Example:
//  Logf("Value = @d\n\r", value)
//
// It is the output driver's responsibility to handle ascii sequences
// Logf sends the character when it is not a format escape
//

// I THINK WE HAVE A STACK OVERFLOW?
VOID KERNEL Logf(const char *fmt, ...)
{
    char printfmt_buffer[MAX_STR_LENGTH_OF_UINT32 + 1];
    BOOL is_signed;

    va_list args;

    va_start(args, fmt);

    for (U32 i=0; fmt[i] != 0; i++)
    {
        if     (fmt[i] == '@') { is_signed = 0;       }
        else if(fmt[i] == '#') { is_signed = 1;       }
        else                   { od(fmt[i]); continue;}

        // The following runs if this is a format character
        i+=2; // Skip the format characters

        switch (fmt[i-1])
        {
        // Print hexadecimal, sign is ignored but should be @ for logical reasons
        case 'x':
            Hex32ToString(va_arg(args, U32), printfmt_buffer);
        break;

        // Print integer, signed or unsigned format
        case 'i':
            if (is_signed) {
                // Well, *is* it negative?
                const S32 value = va_arg(args, S32);
                const BOOL negative = value >= 0;

                if (negative) {
                    // In two's complement, a 4-bit number like 1000
                    // would be -1. Whatever exists without the top bit plus one
                    // is the magnitude, exclusive of sign.
                    const S32 mag = (value & (~(-1))) + 1;

                    if (negative) od('-');

                    // Now we can convert the magnitude to a string and print
                    Uint32ToString(mag, printfmt_buffer);
                    WriteAsciiz(printfmt_buffer);
                } else {
                    Uint32ToString(value, printfmt_buffer);
                    WriteAsciiz(printfmt_buffer);
                }
            }
        break;

        case 's':
            // TODO? Just use write asciiz
        break;

        case '#': od('#');
        break;
        case '@': od('@');
        break;

        default:
            goto FailSilent;
        }
    }

    FailSilent:

    va_end(args);
}

VOID KERNEL FatalError(U32 error_code)
{
}

/*********************************  License  ***********************************

			Copyright (C) 2025 Joey Qytyku

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the ?Software?), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED AS IS, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

************************************  About  ***********************************

This module is a standards-compliant printf implementation written in one single
file with one header, and it targets C99 or higher.

It is tested against the output of the gnu libc printf, which is the reference
implementation used here. Designed to be reentrant.

When compiling with floating point support, a standard library is required. If one does not exist, define them with macros or externally link the appropriate
math functions.

Aside from that, there is no dependency on stdlib functions and a built-in
implementation exists when necessary.

If testing natively, compile with SHARED_PRINTF_TESTING_NATIVE defined.

This printf is a good balance between efficiency and compactness. It may not be ideal for embedded projects if size is the most important, but the compiler does a good job with that regardless.

********************************** How To Use **********************************

This module implements a function called _printf_core. The source package should
include examples of how to implement some of the derivatives.

_printf_core is used to implement every other variant of the printf family.
It works like vsnprintf but with a buffer write callback.

Copy printf.c into your source tree and use another file to implement the
functions needed with the desired function signature.

*******************************************************************************/


#if defined(SHARED_PRINTF_TESTING_NATIVE)
	#define SHARED_PRINTF_ENABLE_FLOAT
	#include <stdio.h>
	#include <stdlib.h>
#endif

#ifdef __MEDIUM__
/*
	Compiling for 16-bit DOS with ia16-gcc. Float is automatically
	disabled.
*/
#endif

// These ones are always going to exist, even if freestanding.
#include <stdarg.h>		/* variadic arguments*/
#include <stddef.h>		/* size_t */
#include <sys/types.h>		/* ssize_t */
#include <stdint.h>		/* type sizes */
#include <limits.h>		/* bit widths */

#include <stdbool.h>

#include <string.h>		/* What to do about this? */

/* A freestanding environment may want float. An OS would not. */
#ifdef ENABLE_FOAT
	#include <float.h>
	#include <math.h>
#endif /* ENABLE_FOAT */

#define STR_(x) #x

/*
BTW ADD -Wstrict-aliasing=2 to kernel
*/

/*
	Returns the number of characters needed to represent a base-10 integer value.
	Uses floating point, but it generates no code so it works when float is
	disabled by the compiler.

	This is the exact and reliable number for both buffer sizes and
	iterations counts, and is truly portable. Works on GCC 4.4.7.
*/
#define DECFIGS(x) ((unsigned)__builtin_ceil(__builtin_log10((double)(x))))

/*
	For consistency, this has to return a value that works for any
	input (useful later?).
*/
#define HEXFIGS(x) ((x)>=16?((sizeof(typeof(x))*8)-__builtin_clzll(x))/4:1)

// Divide but with cieling. Used to calculate octal digits. E.g.
// 32-bit number is 32/3=10.66 but we ceil to get 11 digits.
#define CIELDIV_U(n, d) ((n) % (d) == 0 ? (n)/(d) : (n) / (d) + 1)

/*
	Number of octal digits in a constant number, but it takes a TYPE
	and NOT a number. Reliable and accurate.
*/
#define OCTFIGS(_TYPE) CIELDIV_U(sizeof(_TYPE)*CHAR_BIT, 3)

#ifndef unlikely
	#define unlikely(x) __builtin_expect((x),0)
#endif

#ifdef likely
	#define likely(x)   __builtin_expect((x),1)
#endif

/*******************************************************************************
				Type Definitions
*******************************************************************************/

typedef enum {
	l_NONE = 0,
	l_hh,           // Usually 8-bit
	l_h,            // Usually 16-bit
	l_l,            // long
	l_ll,           // long long
	l_j,            // intmax_t
	l_z,            // size_t
	l_t,		// ptrdiff
	l_L		// Long float type
}lenmod;

// This structure is the full context of a printf_core instance.
// It contains things that are shared with helper routines and are
// not worth making into local variables.
//
// This allows for reentrancy.
//
typedef struct {
	size_t		bytes_left;
	size_t		bytes_printed;
	unsigned int	padding_req;
	unsigned int	precision;

	// The hash modifier
	unsigned char	alternate:1;

	// Add a space in the place of a minus sign.
	// Example: % i
	//
	unsigned char	prepend_space:1;

	unsigned char	plus_sign:1;

	// Boolean to indicate
	unsigned char	leading_zero_pad:1;

	unsigned char	justify:1;

	// Format conversion requested as a character
	char		req_fmt;

	// One of the lenmod options. This goes before the requested format.
	// For example %lu, %li, %lli
	unsigned char	length_mod;

	//
	// Index to the string. Goes one by one except for
	// formats.
	//
	unsigned int inx;

	// This pointer is not null but may be undefined if the results are
	// to be discarded or outputted instantly.
	//
	// It moves up as things are printed.
	//
	char *out_buffer;
}printfctl;


//
// This is called by the core function to write bytes from any buffer to the
// target buffer. NULL must be handled internally by the caller.
//
typedef void (*commit_buffer_f)(printfctl *ctl, const char *b, size_t c);

/*******************************************************************************
			     Conversion functions
*******************************************************************************/

/* NUMBER OF DECIMAL CHARACTERS */
static const size_t	ndc_int		= DECFIGS(INT_MAX),
			ndc_uint	= DECFIGS(UINT_MAX),
			ndc_char	= DECFIGS(CHAR_MAX),
			ndc_uchar	= DECFIGS(UCHAR_MAX),
			ndc_long	= DECFIGS(LONG_MAX),
			ndc_llong	= DECFIGS(LLONG_MAX),
			ndc_ulong	= DECFIGS(ULONG_MAX),
			ndc_ullong	= DECFIGS(ULLONG_MAX),
			ndc_size	= DECFIGS(SIZE_MAX);


/* NUMBER OF OCTAL CHARACTERS */

#if 0 /*__i386__*/
/*
    This code mogs anything that even the latest GCC outputs. If compiling for
    i386, this is used instead of the generic version.

    "Real Programmers Don't Use Pascal" vindicated.
*/
__attribute__((regparm(0), cdecl))
static char *utoa_bw(char *tbuff, unsigned int value)
{
	__asm__ (
	R"(
	push %edi

	movl 16(%esp),%ecx  # ECX = iterations
	movl 12(%esp),%edi   # EDI = buffer, we need it for later
	movl 8(%esp),%eax   # EAX = value

	# If the number is under 65535, reduce iterations to 5 for ~-200 clocks
	cmpl $0xFFFF,%eax
	jae 0f
	shl $1,%ecx
    jmp 0f

	.align  16
0:
	xorl %edx,%edx      # Clear EDX for division
	subl $1,%edi        # Decrement down
	subl $1,%ecx        # Decrement loop counter
	divl %ecx           # EDX now equal to the digit
	add $'0',%dl        # Convert to character
	movb %dl,(%edi)     # Copy to the buffer
	jecxz 1f            # If zero, leave
	jmp 0b
	.align 16
1:

	mov $'0',%eax
	repe scasb
	mov %edi,%eax

	subl 12(%esp),%edi
	mov %edi,%eax

	pop %edi
	ret
	)");
}
#else

static char *utoa_bw(char *tbuff, unsigned int value)
{
	tbuff--;
	*(tbuff) = (char)( ((unsigned)'0') + value % 10 );

	if (value <= 9)
		return tbuff;

	for (unsigned i = 1; i < ndc_uint; i++) {
		tbuff--;
		*(tbuff) = (char)( (unsigned)'0' + (value /= 10) % 10);
	}

	for (unsigned i = 0; i < ndc_uint-1; i++)
		if (*tbuff == '0')
			tbuff++;
		else
			break;

	return tbuff;
}
#endif

static char *ulltoa_bw(char *tbuff, unsigned long long value)
{
	tbuff--;
	*(tbuff) = (char)( (unsigned)'0' + value % 10 );

	if (value <= 9)
		return tbuff;

	for (unsigned i = 1; i < ndc_ullong; i++) {
		tbuff--;
		*(tbuff) = (char)((unsigned)'0' + (value /= 10LL) % 10LL);
	}

	for (unsigned i = 0; i < DECFIGS(ULLONG_MAX)-1; i++) {
		if (*tbuff == '0')
			tbuff++;
		else
			break;
	}
	return tbuff;
}

static unsigned itox
(
	unsigned long long	value,
	unsigned		cap,
	unsigned 		maxdigits,
	char *			buff
)
{
	static const char lookup1[16] =
	{'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
	static const char lookup2[16] =
	{'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

	const char * const lookup = cap ? lookup2 : lookup1;

	unsigned int nonzero_encountered = 0, ret = 0;

	for (int i = (signed)maxdigits-1; i >= 0; i--)
	{
		char digit = lookup[ (value >> ((unsigned)i*4)) & 0xF ];

		if (!nonzero_encountered && digit != '0')
	            nonzero_encountered=1;

		if (nonzero_encountered) {
			buff[ret] = digit;
			ret++;
		}
	}
	return ret;
}

/*
	Returns number of characters generated.

	Forward generating to the buffer.
*/
static unsigned int itoo_fw
(
	unsigned long long int  value,
	char *                  buff
)
{
	unsigned int nonzero_encountered = 0;
	unsigned int ret = 0;

	for (int i = 21; i >= 0; i--) {
		const char digit = (char)( ((value >> (i*3)) & 7) + '0' );
	        if (!nonzero_encountered && digit != '0')
	            nonzero_encountered=1;

		if (nonzero_encountered) {
			buff[ret] = digit;
			ret++;
		}
	}
	return ret;
}

unsigned int custom_atou(const char *s)
{
	unsigned int final = 0;
	unsigned int multiplier = 1;

	if (s == NULL)
		return 0;

	// Find end of the string.
	// USE THIS FOR DIGITS
	size_t len = strlen(s);

	for (size_t i = 0; i < len; i++) {
		final += multiplier * (unsigned)(s[i]-'0');
		multiplier *= 10;
	}
	return final;
}

static void do_pad
(
	printfctl *	ctl,
	commit_buffer_f	cmt,
	const char *	toprint,
	size_t		length
)
{
	size_t pad_width = ctl->padding_req;

	static char b[2] = {' ', '0'};

	if (length >= pad_width || pad_width == 0) {
		cmt(ctl, toprint, length);
		return;
	}

	#define _inspad \
	for (size_t i = 0; i < pad_width - length; i++) { \
		cmt(ctl, b + !!ctl->leading_zero_pad, 1); \
	}

	if (ctl->justify == 0) {
		cmt(ctl, toprint, length);
		_inspad
	}
	else {
		_inspad
		cmt(ctl, toprint, length);
	}
	#undef _inspad

}

static inline unsigned umax(unsigned x, unsigned y)
{
	return x > y ? x : y;
}
static inline unsigned umin(unsigned x, unsigned y)
{
	return x < y ? x : y;
}
static inline unsigned iabs(int a)
{
	return (unsigned)(a < 0 ? -(a) : a);
}
static inline unsigned long lliabs(long long a)
{
	return (unsigned long long) (a < 0 ? -(a) : (unsigned long long)a);
}


static void dup_buffer_with_zeroes_and_print
(
	printfctl *     ctl,
	commit_buffer_f cmt,
	const char *    b,
	unsigned        old_buff_size,
	unsigned        minimum,
	char            prefix
)
{
	unsigned int nz;

	if (old_buff_size >= minimum)
		nz = 0;
	else
		nz = iabs((signed)old_buff_size - (signed)minimum);

	const unsigned int has_prefix = (prefix != 0);

	char b2[has_prefix + nz + old_buff_size];

	memset(b2+has_prefix, '0', nz);
	memcpy(b2+has_prefix + nz, b, old_buff_size);

	if (has_prefix)
		b2[0] = prefix;

	do_pad(ctl, cmt, b2, sizeof b2);
}

// atoi but for a non-null terminated substring.
// Used to extract numbers from printf flags for padding and precision.
//
// index_inc is address of index to the string. Incremented by the number
// of digits found.

static int atou_substring(      const char *	str,
				unsigned int *	index_inc
				)
{
	// Change the byte right after all the numbers that
	// follow the first one to zero and then use atoi.
	// It will be restored later. This is way faster than
	// copying to a buffer.
	// Also, there is no worry about writing past the nullterm and
	// it can save/restore that too.
	int j;

	char buff[ndc_int];

	for (j = 0; str[j] >= '0' && str[j] <= '9'; j++, (*index_inc)++)
		buff[j] = str[j];
	buff[j] = 0;

	return atoi(buff);
}

static void set_fmt_params_defaults(printfctl *ctl)
{
	ctl->padding_req=0;
	ctl->alternate=0;
	ctl->length_mod=l_NONE;
	ctl->prepend_space=0;
	ctl->plus_sign=0;
	ctl->leading_zero_pad=0;
	ctl->precision=0;
	ctl->justify = 1;       // Justify is to the right by default!!!
}

//
// When getting signed integers, it is necessary to sign extend the value
// correctly. Not needed for anything unsigned. In that case only 64/32 matter
// because of stack alignment.
//
static long long int consume_sigint(va_list *v, unsigned l)
{
	switch (l)
	{
		case l_hh:	return (char)va_arg(*v, int);
		case l_h:	return (short)va_arg(*v, int);
		case l_NONE:	return va_arg(*v, int);
		case l_l:	return va_arg(*v, long);
		case l_ll:	return va_arg(*v, long long);
		case l_j:	return va_arg(*v, intmax_t);
		case l_t:
		case l_z:	return va_arg(*v, ptrdiff_t);
		default:	return 0;
	}
/*
	The z flag means a signed version of size_t. This is defined by posix.
	Standard C does not define this for some reason but supports it in
	printf.

	This is strange because there is no standard type to represent it, so
	how would one pass it?

	ptrdiff_t is usually the same as ssize_t on posix. The manpage says
	that it is fine to pass ssize_t to a ptrdiff_t format, but converting
	to a intmax_t may not work as expected since they do not have to be
	the same.

	There is no better way to do this in C. typeof will give a signed number
	and only C++ can remove a qualifier.
*/
}

// Returns the size of a buffer needed to store an integer based on the
// size specifier. DOES NOT INCLUDE SIGN.
//
// This works both ways because the digit counts are the same when using
// twos complement.
//
// l_j prints out (u)intmax_t.          Handled using long long.
// l_z prints out (signed)size_t.       If 64-bit target, 64-bit.
//
static unsigned char intblt[] = {
	[l_hh]  = DECFIGS(UCHAR_MAX	),
	[l_h]   = DECFIGS(USHRT_MAX	),
	[l_NONE]= DECFIGS(UINT_MAX	),
	[l_l]   = DECFIGS(ULONG_MAX	),
	[l_ll]  = DECFIGS(ULLONG_MAX	),
	[l_j]   = DECFIGS(UINTMAX_MAX	),
	[l_z]   = DECFIGS(PTRDIFF_MAX	)
};

// printfctl *ctl
// CHECK THE SIZE SPEC!!!
static void fmt_i(printfctl *ctl, commit_buffer_f cmt, va_list *va)
{
	const	unsigned bl = intblt[ctl->length_mod];
	const	unsigned lm = ctl->length_mod;
		long long v = consume_sigint(va, lm);
	const int has_prefix = (v < 0 || ctl->prepend_space || ctl->plus_sign);
	char *a = NULL;


	char b[ bl + has_prefix ];

	memset(b, '%', sizeof(b));

	if (intblt[lm] <= intblt[l_NONE]) {
		a = utoa_bw(b + sizeof b, iabs(v));
	}
	else {
		a = ulltoa_bw(b + sizeof b, lliabs(v));
	}

	if (v < 0)			a[-1] = '-', a--;
	else if (ctl->plus_sign)	a[-1] = '+', a--;
	else if (ctl->prepend_space)	a[-1] = ' ', a--;

	if (ctl->precision == 0) {
		do_pad(ctl, cmt, a, (b+sizeof(b)) - a);
	}
	else {
		// THis needs to be fixed actually
		dup_buffer_with_zeroes_and_print(
			ctl,
			cmt,
			a,
			((b+sizeof(b)) - a),
			ctl->precision,
			has_prefix ? a[0] : 0
		);
	}
}

static void fmt_u(printfctl *ctl, commit_buffer_f cmt, va_list *va)
{
}

static void fmt_x(printfctl *ctl, commit_buffer_f cmt,va_list *va)
{
	char b[sizeof(long long) * 2];

	static const size_t digits[] = {
		[l_hh]  = 2*sizeof(char),
		[l_h]   = 2*sizeof(short),
		[l_NONE]= 2*sizeof(int),
		[l_l]   = 2*sizeof(long),
		[l_ll]  = 2*sizeof(long long),
		[l_j]   = 2*sizeof(intmax_t),
		[l_z]   = 2*sizeof(size_t)
	};

	// It is safe to limit the precision with a larger than needed number
	// when using a narrow type but this will stack smash for anything
	// larger than 20 because of the long long type.

	// We can still use this for printing hex since the data sizes totally
	// correspond.
	const size_t bufflen = digits[ctl->length_mod];
	unsigned long long int val =
		bufflen > 8U ?
			va_arg(*va, unsigned long long int) :
			va_arg(*va, unsigned int);

	unsigned int chars_gen = itox(
						val,
						ctl->req_fmt == 'X',
						digits[ctl->length_mod],
						b
						);

	// According to cppreference:
	// For integer numbers it is ignored if the precision is explicitly
	// specified.
	// So basically only one can be true and we check for the precision
	// first since it takes precedent.

	if (ctl->precision != 0) {
		// The existing do_pad function is not sufficient for this
		// purpose. It does not allow for the arbitrary length
		// leading zeroes required.
		dup_buffer_with_zeroes_and_print(
			ctl,
			cmt,
			b,
			chars_gen,
			ctl->precision,
			0
		);
	}
	else {
		// Padding with zeroes is automatically handled.
		do_pad(ctl, cmt, b, chars_gen);
	}
}

static void fmt_s(printfctl *ctl, commit_buffer_f cmt, va_list *va)
{
	// If the string is null, it is undefined behavior.
	// For interoperability, we output (null)
	// The string uses the precision field to limit the length

	const char * __restrict str = va_arg(*va, const char *);

	// Not defined, but added for interoperability with code that relies
	// on null strings not failing.
	if (str == NULL) {
		do_pad(ctl, cmt, "(null)", 6);
		return;
	}

	size_t l = strlen(str);

	if (ctl->precision != 0) // Unlikely
		l = l >= ctl->precision ? ctl->precision : l;

	do_pad(ctl, cmt, str, l);
}

#if defined(SHARED_PRINTF_ENABLE_FLOAT)
static void fmt_g(printfctl *ctl, commit_buffer_f cmt, va_list *va)
{
}
static void fmt_G(printfctl *ctl, commit_buffer_f cmt, va_list *va)
{
}
#endif

static void fmt_o(printfctl *ctl, commit_buffer_f cmt, va_list *va)
{
	static size_t digits[] = {
		[l_hh   ] = OCTFIGS(unsigned char),
		[l_h    ] = OCTFIGS(unsigned short),
		[l_NONE ] = OCTFIGS(unsigned int),
		[l_l    ] = OCTFIGS(unsigned long),
		[l_ll   ] = OCTFIGS(unsigned long long),
		[l_j    ] = OCTFIGS(uintmax_t),
		[l_z    ] = OCTFIGS(size_t),
		[l_t    ] = OCTFIGS(ptrdiff_t)
	};
	#undef ODS

	const size_t bufflen = digits[ctl->length_mod];

	const unsigned long long val = bufflen >= digits[l_ll] ?
						va_arg(*va, unsigned long long):
						va_arg(*va, unsigned int);

	char b[bufflen];

	unsigned int chars_gen = itoo_fw(val, b);

	if (ctl->precision != 0) {
		dup_buffer_with_zeroes_and_print(
			ctl,
			cmt,
			b,
			chars_gen,
			ctl->precision,
			0
		);
	}
	else {
		do_pad(ctl, cmt, b, chars_gen);
	}
}
static void fmt_p(printfctl *ctl, commit_buffer_f cmt, va_list *va)
{
	// Implementation-defined pointer format. Normally is print a hex
	// string with a 0x prefix and lowercase letters.
	// We will do (nil) for null pointers and regular 0x format with
	// lower cases.
	// Most modifiers for it are invalid.


}
static void fmt_a(printfctl *ctl, commit_buffer_f cmt, va_list *va)
{
}
static void fmt_A(printfctl *ctl, commit_buffer_f cmt, va_list *va)
{
}
static void fmt_e(printfctl *ctl, commit_buffer_f cmt, va_list *va)
{
}
static void fmt_E(printfctl *ctl, commit_buffer_f cmt, va_list *va)
{
}
static void fmt_n(printfctl *ctl, commit_buffer_f cmt, va_list *va)
{
}

static void fmt_percent(printfctl *ctl, commit_buffer_f cmt, va_list *va)
{
	cmt(ctl, NULL, '%');
}

typedef void (*fmt_handler)(    printfctl *__restrict   ctl,
				commit_buffer_f         cmt,
				va_list *               va
				);

// This may be too large. I may want to reduce the size by basing them off
// the first letter of alphabet.
static const fmt_handler fmt_lookup[] = {
	['i'] = fmt_i,
	['u'] = fmt_u,
	['x'] = fmt_x,  // They are the same and autodetected internally
	['X'] = fmt_x,
	['s'] = fmt_s,
	['d'] = fmt_i,

	#if defined(SHARED_PRINTF_ENABLE_FLOAT)
	['g'] = fmt_g,
	['G'] = fmt_G,
	#endif

	['o'] = fmt_o,
	['p'] = fmt_p,
	['a'] = fmt_a,
	['A'] = fmt_A,
	['e'] = fmt_e,
	['E'] = fmt_E,
	['n'] = fmt_n,
	['%'] = fmt_percent
};

static char is_valid_fmt(char f)
{
	if (fmt_lookup[(unsigned char)f] != NULL) {
		return f;
	}

	return '\0';
}

//
// A format is like an instruction. This is the decode stage.
// the va_list is for the case in which we need use the * prefix.
//
static void set_fmt_params(     printfctl *     ctl,
				const char *    f,
				va_list *       v
				)
{
	// Leave at zero by default so conversions can check if padding
	// needs to be calculated at all.

	// Not setting justify because it will be set if it is needed.

	//
	// The right-justified padding is placed at the beginning of the
	// format specifier. It does not need to be iteratively checked.
	// However, this HAS to be a loop despite the printf format being
	// structured quite well because the # modifier can be inserted
	// anywhere.
	//

	// Local index for better optimization. Later copied to the ctl.
	unsigned int lx = ctl->inx;

	while (1)
	{
		// What we will do here is parse the flags of the format
		// description. There are no characters which can be
		// considered format conversion specifiers that are
		// also flags. The conversion spec is always at the
		// very end of the format.

		// Is this a format specifier? If so, we are done.
		// Can be converted to a lookup as it is based on a char anyway.
		if ( (ctl->req_fmt = is_valid_fmt(f[lx])) != 0 ) {
			lx++;
			break;
		}

		// If we encounter a number with no special prefix, we know
		// that it is the padding count. Justify is already set.
		else if(	(unsigned)f[lx] >  (unsigned)'0' &&
				(unsigned)f[lx] <= (unsigned)'9'
		){
			ctl->padding_req =
				(unsigned)atou_substring(f + lx, &lx);
		}

		// Other things fit into a switch case.
		else
		switch (f[lx])
		{
			case 'h':
				// Probably wrong, refactor?????
				ctl->length_mod = ({unsigned char c;
					if (f[lx+1]=='h') {
						c=l_hh;
						lx+=2;
					}
					else {
						c=l_h;
						lx++;
					}
					c;
				});
			break;

			case 'l':
				ctl->length_mod = ({unsigned char c;
					if (f[lx+1]=='l') {
						c=l_ll;
						lx+=2;
					}
					else {
						c=l_l;
						lx++;
					}
					c;
				});
			break;

			case 'j':
				ctl->length_mod = l_j;
				lx++;
			break;

			case 'z':
				ctl->length_mod = l_z;
				lx++;
			break;

			case 't':
				ctl->length_mod = l_t;
				lx++;
			break;

			case 'L':
				ctl->length_mod = l_L;
				lx++;
			break;

			case '%':
				// This will print a percent sign.
				lx++;
				goto out;

			case '0':
				// Pad using leading zeroes.
				// Only applies to floating point conversions.
				// This works even when another padding is
				// specified.
				// E.g. "%020f", 1.0 would print
				// 0000000000001.000000
				ctl->leading_zero_pad=1;
				// Left justify is not supported by the '0'
				// flag and the '-' should be ignored.
				// This will do that.
				ctl->justify=1;
				lx++;
			break;

			case '-':
				// Justify to the left.
				ctl->justify = 0;
				lx++;
			break;

			case '.':
				lx++; // Bring up by one unconditionally

				// If * comes after, use int arg for precision
				// Dot always comes before something btw.
				// It is done here because ".*" is treated
				// as a full sequence wheile "-" and "*"
				// are separate flags. "-" on its own will
				// change the justification only.

				// Next character is asterick?
				// Set precision using extra int.
				if (f[lx] == '*') {
					ctl->precision = va_arg(*v, unsigned);
					lx++;
				}
				else if (f[lx] >= '1' && f[lx] <= '9') {
					ctl->precision =
						(unsigned)
						atou_substring(f + lx, &lx);
				}
			break;

			case ' ':
				ctl->prepend_space = 1;
				lx++;
			break;

			case '+':
				ctl->plus_sign = 1;
				lx++;
			break;

			case '#':
				// Alternate form
				// Defined per format spec
				ctl->alternate=1;
				lx++;
			break;

			case '*':
				// This is for an asterick encountered on its
				// own. This right-justified by default.

				ctl->padding_req = va_arg(*v, unsigned);
				lx++;
			break;

			default:
				;
		}
	}
	out:

	ctl->inx = lx;
}

/*
	A drawback to this function is that printing single characters is very
	slow. This is visible when the cycles are low on DOSBox.
	A callback must be called for every character, which contains condition
	checks.

	Remember that there are more normal chars that formats.

	Also, I may want to use strchr or some other fast method to locate
	each % sign and copy arbitrary numbers of bytes.
*/
int _printf_core(       printfctl *		ctl,
			char *__restrict        buffer,
			commit_buffer_f         cmt,
			size_t                  count,
                        const char *__restrict  f,
                        va_list                 v
                )
{
	ctl->bytes_left = count;
	ctl->out_buffer = buffer;
	ctl->bytes_printed = 0;
	ctl->inx=0;

	while (f[ctl->inx] != 0)
	{
		if (unlikely(f[ctl->inx] == '%')) {
			ctl->inx++;
			set_fmt_params_defaults(ctl);

			set_fmt_params(ctl, f, &v);
			fmt_lookup[(unsigned char)ctl->req_fmt](ctl, cmt, &v);
		}
		else {
			cmt(ctl, NULL, (size_t)f[ctl->inx]);
			ctl->inx++;
		}
	}

	return (int)ctl->bytes_printed;
}

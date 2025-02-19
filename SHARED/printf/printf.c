//
// Copyright (C) 2025 Joey Qytyku, All rights reserved
//
// This is a standards-compliant printf implementation written in one single
// file with one header that targets C99 and higher.
//
// This is MY printf. There are hundreds like it, but this one is MINE.
//
// Tested against the output of the gnu libc printf, which is the reference
// implementation used here.
//
// When compiling with floating point support, a standard library is required.
// Otherwise, the implementation can be thought of as being in "embedded mode"
// and does not have any float-related features.
//
// Aside from that, there is no dependency on stdlib functions and a built-in
// implementation exists when necessary.
//

//
// If testing natively, compile with TESTING_NATIVE defined.
//

#if defined(TESTING_NATIVE)
#	define ENABLE_FLOAT
#	include <stdio.h>
#	include <stdlib.h>
#else

/*
Freestanding environment where we can freely define printf
We do the efficient version where it is converted to puts() if there are no
additional arguments. This is technically undefined behavior because there
could be formats in the string.
*/

#	define printf(fmt, ...) { \
	if (sizeof(_STR(__VA_ARGS__)<=1))\
		{puts(fmt);} } \
	else	{_printf(fmt, __VA_ARGS__);}

#endif /* defined(TESTING_NATIVE) */



// These ones are always going to exist, even if freestanding.
#include <stdarg.h> /* variadic arguments*/
#include <stddef.h> /* size_t */
#include <sys/types.h> /* ssize_t */
#include <stdint.h> /* Some macros needed for sizes??? */

#include <stdbool.h>
#include <limits.h>

#include <string.h>

/* A freestanding environment may want float. An OS would not. */
#ifdef ENABLE_FOAT
#	include <float.h>
#	include <math.h>
#endif /* ENABLE_FOAT */

#define STR_(x) #x

/*
Returns the number of characters needed to represent a number. x needs to be
a numeric literal. May return more than necessary but will always return
the correct number.

Only safe for buffer sizes.

We can make this work better. Something like int i = "Hello"[0] == 'H'
is perfectly valid.

The implementation could use any representation for the byte sizes.

I need to make improvements so that this handles each case properly.

Also, I may want to define the buffer sizes or character countrs
separately as constants or defines.

Also, bit allocation, totally unrelated but as a note, can be optimized by
performing a backward scan and a forward one, and subtracting the results.
If this is large enough, we found the zone.

*/
#define NDFIGS(x) ((sizeof(char []){STR_(x)})-1)

// Divide but with cieling. Used to calculate octal digits. E.g.
// 32-bit number is 32/3=10.66 but we ceil to get 11 digits.
#define CIELDIV_U(n, d) ((n) % (d) == 0 ? (n)/(d) : (n) / (d) + 1)

#define unlikely(x) __builtin_expect((x),0)
#define likely(x)   __builtin_expect((x),1)

// fmt char option always comes after the flags.

typedef enum {
	l_NONE = 0,
	l_hh,           // Usually 8-bit
	l_h,            // Usually 16-bit
	l_l,            // long
	l_ll,           // long long
	l_j,            // intmax_t
	l_z,            //
	l_t,
	l_L
}lenmod;

//
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

//
// value is promoted to avoid losing the sign when converting narrow
// values.
//
unsigned int libc90_atoi(const char *s)
{
	static const unsigned int lookup[] =
	{1,10,100,1000,10000,100000,1000000,10000000,100000000,1000000000};

	unsigned int final = 0;

	if (s == NULL)
		return 0;

	// Find end of the string.
	size_t len = strlen(s);
        unsigned int n = 0;
	for (size_t i = 0; i < len; i++) {
		if (s[i] == '-') {
			n=1;
			continue;
		}
		else if (s[i] == ' ')
			continue;
		final += lookup[len-i-1] * (unsigned)(s[i]-'0');
	}
	return n ? -final : final;
}

// Buffer should usually be 11 bytes, with one for null terminator
// Returns number of character bytes outputted, or number of digits
// and be used to insert null.
unsigned int libc90_utoa64(unsigned long long int value, char *buff)
{
	static const unsigned long long int lookup[20] =
	{
	10000000000000000000ULL,1000000000000000000ULL,100000000000000000ULL,
	10000000000000000ULL,1000000000000000ULL,100000000000000ULL,
	10000000000000ULL,1000000000000ULL,100000000000ULL,10000000000ULL,
	1000000000ULL,100000000ULL,10000000ULL,1000000ULL,100000ULL,10000ULL,
	1000ULL,100ULL,10ULL,1ULL
	};

	unsigned int i = 0;

	// If we are printing only one digit (quite common) do the simple
	// conversion.
	if (value < 9) {
		buff[i] = (char)((unsigned)value + (unsigned)'0');
		return 1;
	}

	_Bool found_first_zero = 0;

	for (unsigned int j = 0; j < 20; j++) {
		unsigned int d = (unsigned)((value / lookup[j]) % 10ULL);

		// We only care about zeroes if we have encountered the first
		// non-zero digit (except if value is zero).

		if (d == 0 && !found_first_zero) {
			continue;
		}
		// If a non-zero is found, zeroes after
		// matter so first condition is not true.
		else if (d != 0) {
			found_first_zero = 1;
		}
		buff[i++] = (char)((unsigned)'0' + d);
	}
	return i;
}

unsigned int libc90_utoa32(unsigned int value, char *buff)
{
	static const unsigned int lookup[] =
	{
	    1000000000,
	    100000000,
	    10000000,
	    1000000,
	    100000,
	    10000,
	    1000,
	    100,
	    10,
	    1
	};

	// Index to the buffer and counter of significant
	// digits put out.
	unsigned int i = 0;

	if (value < 9) {
		buff[0] = (char)( value + (unsigned)'0' );
		return 1;
	}

	int found_first_nonzero = 0;

	for (unsigned int j = 0; j < 10; j++) {
		unsigned int d = ((value / lookup[j]) % 10U);

		if ((d == 0) && (!found_first_nonzero)) {
			continue;
		}
		else if (d != 0) {
			found_first_nonzero = 1;
		}

		buff[i] = (char)( (unsigned)'0' + d );
		i++;

	}
	return i;
}

// There is no need for iterating when using precision on integer or hex.
// Just offset from the end and then check for zeroes if needed. Check if
// the padding is set.

//
// These functions only convert the magnitude and do not output a sign.
//
unsigned int libc90_itoa32(int value, char *buff)
{
	if (value < 0)
		return libc90_utoa64((unsigned) (-value), buff);
	else
		return libc90_utoa64((unsigned) ( value), buff);
}

unsigned int libc90_itoa64(long long int value, char *buff)
{
	if (value < 0LL)
		return libc90_utoa64((unsigned long long) (-value), buff);
	else
		return libc90_utoa64((unsigned long long) ( value), buff);
}

static void get_float_digits(float v, unsigned int *out)
{
	out[0] = (unsigned int)v;
	out[1] = NDFIGS((unsigned int)v) * (v - (unsigned int)v);
}

// long double is the longest possible floating point type. We do not
// actually know its size. It will be at least as long as double.
// On x86 it will certainly be 80-bit, but on aarch64 it is 64-bit.
//
// C does not support anything shorter than a float so we only have three
// variants to worry about.
//
// We can safely use memory allocation for this since it wont be used on KRNL.
//
void libc90_ftoa(long double value, char *buff)
{
	// We get the
}

unsigned libc90_itox
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

	for (int i = (signed)maxdigits-1; i >= 0; i--) {
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

// Limiting the digits is more of a performance optimization.
// It reduces iterations but simple and'ing can do the trick for deciding
// the proper of the value.
unsigned int libc90_itoo(	unsigned long long int  value,
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

// printf padding is actually dumber than I originally thought it was.
// It does not account for the current position of the cursor relative to the
// line.
//
// Padding is in relation to ONLY the thing being printed. To handle padding
// with more than one thing, one would need to divide the value of the full
// line area for both formats.
//
// For example:
// printf("%-40s%40s", action, status);
//
// This could print something resembling:
//      Loading data                                                    [OK]
//
// If the thing does not fit, well it just doesn't and it gets printed normally.
//
// Also, if including the number's sign with the padding, we just decrease the
// width of the positive value by 1 so an extra space is printed.
//
//
static void do_pad(     printfctl *     ctl,
			commit_buffer_f cmt,
			const char *    toprint,
			size_t          length
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

static inline unsigned int max(unsigned int x, unsigned int y)
{
	return x > y ? x : y;
}

static inline unsigned int min(unsigned int x, unsigned int y)
{
	return x < y ? x : y;
}

static inline unsigned int libc90_iabs(int a)
{
	return (unsigned)(a < 0 ? -(a) : a);
}

static void dup_buffer_with_zeroes_and_print(   printfctl *     ctl,
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
		nz = libc90_iabs((signed)old_buff_size - (signed)minimum);

	const unsigned int has_prefix = (prefix != 0);

	char b2[has_prefix + nz + old_buff_size];

	memset( b2+has_prefix, '0', nz);
	memcpy( b2+has_prefix + nz, b, old_buff_size);

	if (has_prefix)
		b2[0] = prefix;

	do_pad(ctl, cmt, b2, sizeof b2);
}

// atoi but for a non-null terminated substring.
// Used to extract numbers from printf flags for padding and precision.
//
// index_inc is address of index to the string. Incremented by the number
// of digits found.

static int atoi_substring(      const char * __restrict         str,
				unsigned int * __restrict       index_inc
				)
{
	// Change the byte right after all the numbers that
	// follow the first one to zero and then use atoi.
	// It will be restored later. This is way faster than
	// copying to a buffer.
	// Also, there is no worry about writing past the nullterm and
	// it can save/restore that too.
	int j;

	char buff[11];

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
		case l_ll:	return va_arg(*v, long long int);
		case l_j:	return va_arg(*v, intmax_t);
		case l_z:	return va_arg(*v, ssize_t);
		default:	return 0;
	}
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
static size_t integer_bufflen_lookup[] = {
	[l_hh]  = NDFIGS(CHAR_MAX),
	[l_h]   = NDFIGS(SHRT_MAX),
	[l_NONE]= NDFIGS(INT_MAX),
	[l_l]   = NDFIGS(LONG_MAX),
	[l_ll]  = NDFIGS(LLONG_MAX),
	[l_j]   = NDFIGS(INTMAX_MAX),
	[l_z]   = NDFIGS(SSIZE_MAX)
};

// For memcpy in clib, use a compound expression and if statements. They should
// operate on 100% constant values and use features related to that.
//
// We still have to use a macro.
//

// printfctl *ctl
// CHECK THE SIZE SPEC!!!
static void fmt_i(printfctl *ctl, commit_buffer_f cmt, va_list *va)
{
	// This format is modified by several of the size specifiers
	// and the negative for space and plus sign modifiers.
	// "% +i" is not possible because both would have a sign, making it
	// pointless.

	// Precision determines how many characters must be printed, and
	// zeroes are added if needed.

	const unsigned int bufflen = integer_bufflen_lookup[ctl->length_mod];

	char inc_sign = 0;

	char _b[bufflen+1];

	// This will hold only the MAGNITUDE and not the sign.
	// b[0] is the first digit.
	char *b = _b+1;

	// Get a sign extended value. We cannot get a long long on a 32-bit
	// system because it would consume two integers from the stack
	// and corrupt everything.
	const long long int val = consume_sigint(va, ctl->length_mod);

	const unsigned int chars_gen =
		bufflen > 10
			?       libc90_itoa64(val, b):
				libc90_itoa32((int)val, b);

	inc_sign = ({char x;
		if (ctl->plus_sign)             x = val >= 0 ? '+':'-';
		else if (ctl->prepend_space)    x = val >= 0 ? ' ':'-';
		else                            x = val >= 0 ?  0 :'-';
		x;
	});

	_b[0] = inc_sign;

	if (ctl->precision != 0) {
		dup_buffer_with_zeroes_and_print(
			ctl,
			cmt,
			b,
			chars_gen,
			ctl->precision,
			inc_sign
			);
	}
	else {
		do_pad( ctl,
			cmt,
			inc_sign != 0 ? _b : b,
			chars_gen + (inc_sign!=0)
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

	unsigned int chars_gen = libc90_itox(
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

static void fmt_g(printfctl *ctl, commit_buffer_f cmt, va_list *va)
{
}
static void fmt_G(printfctl *ctl, commit_buffer_f cmt, va_list *va)
{
}

static void fmt_o(printfctl *ctl, commit_buffer_f cmt, va_list *va)
{
	#define ODS(_TYPE) CIELDIV_U(sizeof(_TYPE)*CHAR_BIT, 3)
	static size_t digits[] = {
		[l_hh   ] = ODS(unsigned char),
		[l_h    ] = ODS(unsigned short),
		[l_NONE ] = ODS(unsigned int),
		[l_l    ] = ODS(unsigned long),
		[l_ll   ] = ODS(unsigned long long),
		[l_j    ] = ODS(uintmax_t),
		[l_z    ] = ODS(size_t),
		[l_t    ] = ODS(ptrdiff_t)
	};
	#undef ODS

	const size_t bufflen = digits[ctl->length_mod];

	const unsigned long long val = bufflen >= digits[l_ll] ?
						va_arg(*va, unsigned long long):
						va_arg(*va, unsigned int);

	char b[bufflen];

	unsigned int chars_gen = libc90_itoo(val, b);

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
	['g'] = fmt_g,
	['G'] = fmt_G,
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
	// Most common ones go first
	// static const char pf[] = "iuxXsdgGopaAeEn";

	if (fmt_lookup[(unsigned char)f] != NULL) {
		return f;
	}
	// for (unsigned int i = 0; i < sizeof(pf); i++) {
	// 	if (pf[i] == f)
	// 		return f;
	// }
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
				(unsigned)atoi_substring(f + lx, &lx);
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
						atoi_substring(f + lx, &lx);
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
				printf("Unrecognized flag: {%c\n}",f[lx]);
		}
	}
	out:

	ctl->inx = lx;
}


// Buffer can be null, but the commit callback must handle this. That is not
// done here.
//
//
//
int _printf_core(       printfctl *__restrict   ctl,
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
		if (f[ctl->inx] == '%') {
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

// For YT: MS-PAINT STYLE COMICS? GC edit for arch?

// Add support for duplicating characters?
// Also this can be changed to work with VGA characters.

#ifdef TESTING_NATIVE
#define PUTCHAR_COMMIT _putchar_commit
static void _putchar_commit(
	printfctl *	ctl,
	const char *	buff,
	size_t		count
	)
{
	if (buff == NULL) {
		fputc((char)count, stderr);
		ctl->bytes_printed++;
		ctl->bytes_left--;
	}
	else {
		fwrite(buff, count, 1, stderr);
		ctl->bytes_printed += count;
		ctl->bytes_left    -= count;
	}
}

#elif defined(PUTCHAR_COMMIT)

#else
# warning "Define a static function called _putchar_commit before including"
# warning "the implementation."
# error "-- ABORT --"

#endif

//
// The buffer commit procedure must:
// - Move the `out_buffer` pointer according to the characters printed
// - Decrease `bytes_left` for each byte committed
//
// The buffer pointer must be moved even if the results are not actually
// copied. This is because pointer arithmetic is used to calculate the number
// of printed bytes.
//
//
//

//
// The buffer commit procedure must:
// - Move the `out_buffer` pointer according to the characters printed.
// - Decrease `bytes_left` for each byte committed. This has to actually
//   be accurate.
//
// The buffer pointer must be moved even if the results are not actually
// copied. This is because pointer arithmetic is used to calculate the number
// of printed bytes.
//

static void _byte_buff_commit(  printfctl *     ctl,
                                const char *    b,
                                size_t          c
                                )
{
        if (unlikely(ctl->out_buffer == NULL)) {
                // We want the return value of snprintf to be the number of
                // characters that were supposed to be printed. It can be
                // higher than the bounds.
                // The currect calculation does not depend on the initial
                // count minus the number left to get the total printed because
                // that is not what this is.

                // Basically, just move the buffer forward for each character.
                // Even if it overflows past the restrict count, it will be
                // used to get the return value.
                ctl->bytes_printed += c;
        }
        else {
		const char *actual_ibuff = b == NULL ? (const char*)&c : b;
                size_t actual_size = b == NULL ? 1 : min(c, ctl->bytes_left);

                memcpy(ctl->out_buffer, b, actual_size);
                ctl->out_buffer += actual_size;
                ctl->bytes_printed += actual_size;
                ctl->bytes_left -= actual_size;
        }
}

int _printf(const char *__restrict f, ...)
{
	va_list args;
	va_start(args, f);

	printfctl ctl;

	int r = _printf_core(
		&ctl,
		NULL,
		PUTCHAR_COMMIT,
		SIZE_MAX,
		f,
		args
	);

	va_end(args);
	return r;
}

int _snprintf(char *s, size_t n, const char *__restrict f, ...)
{
	va_list args;
	printfctl ctl;

	va_start(args, f);

	int r = _printf_core(&ctl, s, _byte_buff_commit, n, f, args);

	// Insert null terminator
	s[r] = 0;

	// The return value if printf is updated according to the characters
	// actually generated. If buffer commit wants to chop them out, it can.
	// printf will still give the correct value.
	//
	// If the number returned by printf is bigger than n, we return
	// negative.

	if ((size_t)r > n) {
		va_end(args);
		return -1;
	}
	else {
		va_end(args);
		return r;
	}
}

// Note: streams require callbacks to accept reads and writes. This allows
// for a correctly working stdin/stdout. I saw this in the
// standard header myself. However, it may need to be a
// special case for thread safety.
#ifdef TESTING_NATIVE
int main(void)
{
	char b[1024];

	_printf("%.4x\n", 256);
	printf("%.4x\n", 256);

	//
	// There is a difference between the DJGPP printf output and the
	// GCC/Mac one. This is not a regression or error on my part, the OTHER
	// printf is showing an abberation.
	//
	// In this case, I think DJGPP's library has a bug. It abberates from
	// a standards-compliant one.
	//
	// Should send a report.
	//

	return 0;
}
#endif

/*********************************  License  ***********************************

			Copyright (C) 2025 Joey Qytyku

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the ?Software?), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED AS IS, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
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
It works like vsnprintf but with buffer write callbacks.

Copy printf.c into your source tree and use another file to implement the
functions needed with the desired function signature. Include printf.c into the
file that implements the interfaces.

See readme in the SHARED folder (if available) to view all macro options.

*******************************************************************************/

// If we are just testing on a hosted C compiler with a library, include the
// C headers.
#if defined(SHARED_PRINTF_TESTING_NATIVE)
	#define SHARED_PRINTF_ENABLE_FLOAT
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
#endif

// These ones are always going to exist, even if freestanding.
#include <stdarg.h>		/* variadic arguments*/
#include <stddef.h>		/* size_t */
#include <stdint.h>		/* type sizes */
#include <limits.h>		/* bit widths */

#include <stdbool.h>

/* A freestanding environment may want float. An OS would not. */
#ifdef ENABLE_FOAT
	#include <float.h>
	#include <math.h>
#endif /* ENABLE_FOAT */

/* Use? */
#define max(a,b) ({typeof(a) _a = (a), _b = (b); _a > _b ? _a : _b; })

#define STR_(x) #x

/*
	Returns the number of characters needed to represent a base-10 integer value.
	Uses floating point, but it generates no code so it works when float is
	disabled by the compiler.

	This is the exact and reliable number for both buffer sizes and
	iterations counts, and is truly portable. Works on GCC 4.4.7.
*/
#define DFIG(x) ( (unsigned)__builtin_ceil(__builtin_log10( (double)(x))) )

/*
	For consistency, this has to return a value that works for any
	input (useful later?).
*/
#define XFIG(x) ((x)>=16?((sizeof(typeof(x))*CHAR_BIT)-__builtin_clzll(x))/4:1)

// Divide but with cieling. Used to calculate octal digits. E.g.
// 32-bit number is 32/3=10.66 but we ceil to get 11 digits.
#define CIELDIV_U(n, d) ((n) % (d) == 0 ? (n)/(d) : (n) / (d) + 1)

/*
	Number of octal digits in a constant number, but it takes a TYPE
	and NOT a number. Reliable and accurate.
*/
#define OFIG(_TYPE) CIELDIV_U(sizeof(_TYPE)*CHAR_BIT, 3)

#ifndef unlikely(x)
	#define unlikely(x) __builtin_expect((x),0)
#endif

#ifdef likely(x)
	#define likely(x)   __builtin_expect((x),1)
#endif

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

struct _pfc;

typedef void (*commit_buffer_f)(struct _pfc *ctl, const char *b, size_t c);
typedef void (*dupch_f)(struct _pfc *ctl, char ch, size_t n);

typedef void (*fmt_handler)(struct _pfc * ctl);

struct _pfc {
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

	commit_buffer_f cmt;
	dupch_f dup;
	va_list *v;
	/* Octal needs the most digits. */
	char buff[32];
};

typedef struct _pfc printfctl;

static const size_t
ndc_int		= DFIG(INT_MAX),
ndc_uint	= DFIG(UINT_MAX),
ndc_char	= DFIG(CHAR_MAX),
ndc_uchar	= DFIG(UCHAR_MAX),
ndc_ushort	= DFIG(USHRT_MAX),
ndc_short	= DFIG(SHRT_MAX),
ndc_long	= DFIG(LONG_MAX),
ndc_llong	= DFIG(LLONG_MAX),
ndc_ulong	= DFIG(ULONG_MAX),
ndc_ullong	= DFIG(ULLONG_MAX),
ndc_intmax	= DFIG(INTMAX_MAX),
ndc_uintmax	= DFIG(UINTMAX_MAX),
ndc_size	= DFIG(SIZE_MAX),
ndc_ssize	= DFIG(SIZE_MAX/2),
ndc_ptrdiff	= DFIG(PTRDIFF_MAX),
ndc_uptrdiff	= DFIG(PTRDIFF_MAX/2-1);

#define _abs(T, a) (unsigned T)({ unsigned T r = a < 0 ? -a : a; r; })

static const unsigned char uint_iters_tab[8] = {
	[l_hh]   = ndc_uchar,
	[l_h]    = ndc_ushort,
	[l_NONE] = ndc_uint,
	[l_l]    = ndc_ulong,
	[l_ll]   = ndc_ullong,
	[l_z]    = ndc_size,
	[l_t]    = ndc_uptrdiff,
	[l_j]    = ndc_uintmax
};

static const unsigned char int_iters_tab[8] = {
	[l_hh]   = ndc_char,
	[l_h]    = ndc_short,
	[l_NONE] = ndc_int,
	[l_l]    = ndc_long,
	[l_ll]   = ndc_llong,
	[l_z]    = ndc_ssize,
	[l_t]    = ndc_ptrdiff,
	[l_j]    = ndc_intmax
};

static inline unsigned umax(unsigned x, unsigned y) { return x > y ? x : y; }
static inline unsigned umin(unsigned x, unsigned y) { return x < y ? x : y; }

static unsigned max_minus_min(unsigned int a, unsigned int b)
{
	return (unsigned)( a < b ? b - a : a - b );
}

/*
Potential problem: long is not fetched by its own method.
Also, I am assuming that long long is the same as intmax_t

On the sysv abi, this has basically no effect.
*/

void convert_int(printfctl *pc)
{
	static const char pfx_tab[] = {0,'-','+','-',' ','-'};

	unsigned iters = int_iters_tab[pc->length_mod];
	unsigned N = 0;

	char *pb = pc->buff + sizeof(pc->buff);

	memset(pc->buff, '0', sizeof(pc->buff));

	if (iters <= ndc_int) {
		int v = va_arg(*pc->v, int);
		unsigned a = _abs(int, v);
		N = v < 0;

		for (unsigned i = 0; i < iters; i++) {
			pb--;
			*pb = (a % 10U) + '0';
			a /= 10U;
		}
	}
	else {
		long long v = va_arg(*pc->v, long long);
		unsigned long long a = _abs(long long, v);
		N = v < 0;

		for (unsigned i = 0; i < iters; i++) {
			pb--;
			*pb = (a % 10) + '0';
			a /= 10;
		}
	}

	// Find first non-zero character in the sequence
	// pb is at the last character generated, regardless of the type
	// so redundant iterations can be avoided.
	for (unsigned i = 0; i < sizeof(pc->buff); i++) { /*-1?*/
		if (*pb != '0')
			break;
		pb++;
	}

	/*
	OUTPUT SECTION
	*/

	unsigned chars_gen = (unsigned)((pc->buff + sizeof pc->buff) - pb);

	char pfxch = pfx_tab[pc->plus_sign*2+pc->prepend_space*4+N];

	if (chars_gen == 0) chars_gen = 1;

	if (unlikely(pc->leading_zero_pad)) {
		/*
		Leading zero padding is a totally unique scenario that
		I do not want to bloat the most common code path with.
		Precision is redundant in this case and we can rely on
		this to simplify things. Integers also cannot get
		trailing zeroes for obvious reasons.

		The only options relevant now are:
		- +	(plus sign is used if positive)
		- space (space is used if positive)
		- Nothing

		The sign is the first thing to be printed. It is one
		character that can be substituted entirely with a
		zero char.
		*/

		// Is padding necessary? If so, we just print.
		// This includes the prefix character
		if (chars_gen + (pfxch!=0) >= pc->padding_req) {
			// Print the number as-is
			pc->dup(pc, pfxch, pfxch != 0);
			pc->cmt(pc, pb, chars_gen);
		}
		else {
			/*
			In this case, padding is necessary.
			*/
			pc->dup(pc, pfxch, 1);
			pc->dup(
				pc,
				'0',
				pc->padding_req - chars_gen - (pfxch!=0)
			);
			pc->cmt(pc, pb, chars_gen);
		}
	}
	/* NOT USING ZEROES TO PAD, PADDING WITH SPACES */
	else {
		if (pc->justify == 1) {
			size_t nz =
			pc->precision > chars_gen ? pc->precision-chars_gen
			: 0;

			if (pc->padding_req != 0 && pc->padding_req > chars_gen+nz) {
				pc->dup(
					pc,
					' ',
					pc->padding_req - chars_gen - nz
				);
			}

			pc->dup(pc, pfxch, pfxch != 0);
			pc->dup(pc, '0', nz);

			pc->cmt(pc, pb, chars_gen);
		}
		else {
			/*
			Justifying to the left is much simpler as the prefix
			can be outputted unconditionally.
			*/
			pc->dup(pc, '-', N);
			pc->cmt(pc, pb, chars_gen);
			pc->dup(
				pc, ' ',
				max(chars_gen, pc->padding_req)-chars_gen-N
			);
		}
	}
}

/*
	Prints a buffer with no sign prefix and accounts for zero padding if
	needed. Zero padding makes no sense for a string but does no harm.

	Works for %u,x,s.
*/
// __attribute__((noinline))
static void print_pad_nopfx_buff(
	printfctl *pc, size_t chars_gen, const char *pb)
{
	if (unlikely(pc->leading_zero_pad)) {
		if (chars_gen >= pc->padding_req) {
			pc->cmt(pc, pb, chars_gen);
		}
		else {
			pc->dup(
				pc,
				'0',
				pc->padding_req - chars_gen
			);
			pc->cmt(pc, pb, chars_gen);
		}
	}
	else {
		if (pc->justify == 1) {
			size_t nz =
			pc->precision > chars_gen ? pc->precision-chars_gen
			: 0;

			if (pc->padding_req != 0 && pc->padding_req > chars_gen+nz) {
				pc->dup(
					pc,
					' ',
					pc->padding_req - chars_gen - nz
				);
			}

			pc->dup(pc, '0', nz);
			pc->cmt(pc, pb, chars_gen);
		}
		else {
			pc->cmt(pc, pb, chars_gen);
			pc->dup(
				pc, ' ',
				max(chars_gen, pc->padding_req)-chars_gen
			);
		}
	}
}

void convert_uint(printfctl *pc)
{
	unsigned iters = uint_iters_tab[pc->length_mod];

	char *pb = pc->buff + sizeof(pc->buff);

	memset(pc->buff, '0', sizeof(pc->buff));

	if (iters <= ndc_int) {
		unsigned a = va_arg(*pc->v, unsigned);

		for (unsigned i = 0; i < iters; i++) {
			pb--;
			*pb = (a % 10U) + '0';
			a /= 10U;
		}
	}
	else {
		unsigned long long a = va_arg(*pc->v, unsigned long long);

		for (unsigned i = 0; i < iters; i++) {
			pb--;
			*pb = (a % 10) + '0';
			a /= 10;
		}
	}

	for (unsigned i = 0; i < sizeof(pc->buff); i++) { /*-1?*/
		if (*pb != '0')
			break;
		pb++;
	}

	size_t chars_gen = (unsigned)((pc->buff + sizeof pc->buff) - pb);

	if (chars_gen == 0) chars_gen = 1;

	print_pad_nopfx_buff(pc, chars_gen - 2, pb);
}

static unsigned itox
(
	uintmax_t		value,
	unsigned		cap,
	unsigned 		maxdigits,
	char *			buff
)
{
	// Pack these together and align to cache boundary?
	static const char lookup1[16] =
	{'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
	static const char lookup2[16] =
	{'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

	const char * const lookup = cap ? lookup2 : lookup1;

	unsigned int nze = 0, ret = 0;

	for (int i = (signed)maxdigits-1; i >= 0; i--)
	{
		char digit = lookup[ (value >> ((unsigned)i*4)) & 0xF ];

		if (!nze && digit != '0')
		nze=1;

		if (nze) {
			buff[ret] = digit;
			ret++;
		}
	}
	return ret;
}

static void convert_hex(printfctl *pc)
{
	/* size_t is actually better, based on compiler output. */
	static const size_t digits[] = {
		[l_hh]  = 2*sizeof(unsigned char),
		[l_h]   = 2*sizeof(unsigned short),
		[l_NONE]= 2*sizeof(unsigned int),
		[l_l]   = 2*sizeof(unsigned long),
		[l_ll]  = 2*sizeof(unsigned long long),
		[l_j]   = 2*sizeof(uintmax_t),
		[l_z]   = 2*sizeof(size_t)
	};

	// Handle %p here too!
	// Also the # modifier puts 0x

	/* Should I just use the common buffer? */
	char b[sizeof(uintmax_t)*2 + 2];

	uintmax_t val = 0;

	switch (pc->length_mod)
	{
		case l_hh:
		case l_h:
		case l_NONE:
			val = (uintmax_t)va_arg(*pc->v, unsigned);
		break;
		case l_l:val= (uintmax_t)va_arg(*pc->v, unsigned long);
		break;
		case l_ll:val=(uintmax_t)va_arg(*pc->v, unsigned long long);
		break;
		case l_j:val =(uintmax_t)va_arg(*pc->v, uintmax_t);
		break;
		case l_z:val =(uintmax_t)va_arg(*pc->v, size_t);
		break;
		default: val =(uintmax_t)va_arg(*pc->v, uintptr_t);
	}

	unsigned chars_gen = itox(
		val,
		pc->req_fmt == 'X',
		digits[pc->length_mod],
		b
	);
	if (pc->alternate) {
		if (pc->leading_zero_pad) {
			pc->dup(pc, '0', 1);
			pc->dup(pc, pc->req_fmt, 1);

			// The 0x is considered part of the padding
			// If the number fits in the region, the output will
			// be equal to the padding in length.
			// This means that the padding must be set to -2 of the
			// requested amount.

			pc->padding_req=pc->padding_req<2 ? 2:pc->padding_req-2;

			print_pad_nopfx_buff(pc, chars_gen, b);
		}
		else {
			int spaces = (int)pc->padding_req - (int)chars_gen - 2;
			if (spaces <= 0) {
				goto JustPrint;
			}

			pc->dup(
				pc,
				' ',
				pc->padding_req - chars_gen - 2
			);
			goto JustPrint;
		}
	}
	return ;

	JustPrint:
	pc->dup(pc, '0', 1);
	pc->dup(pc, pc->req_fmt, 1);
	pc->cmt(pc, b, chars_gen);

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

// Still untested.
static unsigned int custom_atou(const char *s)
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

static int atou_substring(      const char *	str,
				unsigned int *	index_inc
				)
{
	int j;

	char buff[ndc_int];

	for (j = 0; str[j] >= '0' && str[j] <= '9'; j++, (*index_inc)++)
		buff[j] = str[j];
	buff[j] = 0;

	return atoi(buff);
}

// printfctl *ctl
// CHECK THE SIZE SPEC!!!
static void fmt_i(printfctl *ctl)
{
	convert_int(ctl);
}

static void fmt_u(printfctl *ctl)
{
	convert_uint(ctl);
}

static void fmt_x(printfctl *ctl)
{
	convert_hex(ctl);
}

static void fmt_s(printfctl *pc)
{
	// Wide strings are not supported btw.

	const char * s = va_arg(*pc->v, const char *__restrict);

	const size_t len = strlen(s);

	const size_t actual = len >= pc->precision ? pc->precision : len;

	// I do not want to have the helper act on the precision or it will
	// insert pointless zero characters like for a number.
	// All we use precision for is to get the appropriate length.
	pc->precision = 0;

	print_pad_nopfx_buff(pc, actual, s);
}

#if defined(SHARED_PRINTF_ENABLE_FLOAT)
static void fmt_g(printfctl *ctl)
{
}
static void fmt_G(printfctl *ctl)
{
}
#endif

static void fmt_o(printfctl *ctl)
{
}

static void fmt_p(printfctl *ctl)
{
	// Implementation-defined pointer format. Normally is print a hex
	// string with a 0x prefix and lowercase letters.
	// We will do (nil) for null pointers and regular 0x format with
	// lower cases.
	// Most modifiers for it are invalid.
}
static void fmt_a(printfctl *ctl)
{
}
static void fmt_A(printfctl *ctl)
{
}
static void fmt_e(printfctl *ctl)
{
}
static void fmt_E(printfctl *ctl)
{
}
static void fmt_n(printfctl *ctl)
{
}

static void fmt_percent(printfctl *ctl)
{
	ctl->dup(ctl, '%', 1);
}

// Find a simpler way to do this, it's inefficient.
// A valid format is always alphabetical, by the way.
static const fmt_handler fmt_lookup[127] = {
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
	['%'] = fmt_percent,
};

static char is_valid_fmt(char f)
{
	return fmt_lookup[(unsigned)f] != 0 ? f : '\0';
}

static void set_fmt_params_defaults(printfctl *ctl)
{
	/* No worries, most of these can be write-combined */
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
// A format is like an instruction. This is the decode stage.
// the va_list is for the case in which we need use the * prefix.
//
static void set_fmt_params(     printfctl *     ctl,
				const char *    f
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
		else switch (f[lx])
		{
			case 'h':
				// Probably wrong, refactor????????
				ctl->length_mod = ({unsigned char c;
					if (f[lx+1]=='h'){
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
					if (f[lx+1]=='l')
						c=l_ll,
						lx+=2;
					else
						c=l_l,
						lx++;
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
					ctl->precision = va_arg(*ctl->v, unsigned);
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

				ctl->padding_req = va_arg(*ctl->v, unsigned);
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
int _printf_core
(
	printfctl *		ctl,
	char *  __restrict	buffer,
	commit_buffer_f		cmt,
	dupch_f			_dup,
	size_t			count,
	const char *__restrict	f,
	va_list			v
)
{
	ctl->bytes_left = count;
	ctl->out_buffer = buffer;
	ctl->bytes_printed = 0;
	ctl->inx=0;
	ctl->cmt = cmt;
	ctl->dup = _dup;
	ctl->v   = &v;

	while (f[ctl->inx] != 0)
	{
		if (unlikely(f[ctl->inx] == '%')) {
			ctl->inx++;

			set_fmt_params_defaults(ctl);
			set_fmt_params(ctl, f);

			fmt_lookup[(unsigned)ctl->req_fmt](ctl);
		}
		else {
			cmt(ctl, NULL, (size_t)f[ctl->inx]);
			ctl->inx++;
		}
	}

	return (int)ctl->bytes_printed;
}

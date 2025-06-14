/*********************************  License  ***********************************

						Copyright (C) 2025 Joey Qytyku

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the ?Software?), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

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

When compiling with floating point support, a standard library is required.
If one does not exist, define them with macros or externally link the
appropriate math functions.

Aside from that, there is no dependency on stdlib functions and a built-in
implementation exists when necessary.

If testing natively, compile with SHARED_PRINTF_TESTING_NATIVE defined.

This printf is a good balance between efficiency and compactness.
It may not be ideal for embedded projects if size is the most important, but
the compiler does a good job with that regardless.

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

// For 16-bit support (for whatever reason, mainly proof of concept)
// size_t can be 16-bit (one segment-1). This means a theoretical ssize_t
// which is not official C but it still mandated for printf as a signed size_t
// (we cannot just typedef that btw).
//
// Problem is ssize_t and ptrdiff_t are not the same in this environemnt because
// ptrdiff_t must be 17-bit per the standard. That is because the start-end
// of a segment must be negative and equal 65536, which is not representible
// in 16 bits.
//
// This is only for GCC/clang. Otherwise, define these manually.
//
#if __SIZEOF_SIZE_T__ == 2
	typedef int16_t _ssize_t;
	typedef uint32_t _uptrdiff_t;

#elif __SIZEOF_SIZE_T__ == __SIZEOF_INT__
	typedef int _ssize_t;
	typedef unsigned int _uptrdiff_t;

#elif __SIZEOF_SIZE_T__ == __SIZEOF_LONG__
	typedef long _ssize_t;
	typedef unsigned long _uptrdiff_t;

#elif __SIZEOF_SIZE_T__ == __SIZEOF_LONG_LONG__
	typedef long long _ssize_t;
	typedef unsigned long long _uptrdiff_t;

#elif __SIZEOF_SIZE_T__ == __SIZEOF_SHORT__
	// In this case, we already know that short is not 16-bit.
	// It is required to be 16-bit. No implementation will make it any less.
	// So this is for the off chance that it is something else.
	typedef short _ssize_t;
	typedef unsigned short _uptrdiff_t;

#endif

/* A freestanding environment may want float. An OS would not. */
#ifdef ENABLE_FOAT
	#include <float.h>
	#include <math.h>
#endif /* ENABLE_FOAT */

/* Use? */
#define max(a,b) ({typeof(a) _a = (a), _b = (b); _a > _b ? _a : _b; })

#define OFIG(T) (((sizeof(T)*CHAR_BIT)+1)/3)

#ifndef unlikely
	#define unlikely(x) __builtin_expect((x),0)
#endif

#ifndef likely
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

// FLAG: Can I use a start/end pointer C++ style?
struct _pfc {
	unsigned bytes_left;
	unsigned bytes_printed;
	unsigned padding_req;
	unsigned precision;

	// The hash modifier

	union {
		unsigned char zero_default_flags;
		struct {
			unsigned char alternate:1;
			// Add a space in the place of a minus sign.
			// Example: % i
			//
			unsigned char	prepend_space:1;
			unsigned char	plus_sign:1;
			unsigned char	leading_zero_pad:1;
			unsigned char	fmtchar_capital:1;
		};
	};

	unsigned char justify:1; // True by default

	// Format conversion requested as a character
	char req_fmt;

	// One of the lenmod options. This goes before the requested format.
	// For example %lu, %li, %lli
	unsigned char length_mod;

	// This pointer is not null but may be undefined if the results are
	// to be discarded or outputted instantly.
	//
	// It moves up as things are printed.
	//
	char *out_buffer;

	commit_buffer_f cmt;
	dupch_f dup;

	void *current_arg;

	char buff[32];
};

typedef struct _pfc printfctl;

// FLAG: Do I have another like this?
#define _abs(T, a) (unsigned T)({ unsigned T r = a < 0 ? -a : a; r; })

// FLAG: These two are not being used. Remove?
static inline unsigned umax(unsigned x, unsigned y) { return x > y ? x : y; }
static inline unsigned umin(unsigned x, unsigned y) { return x < y ? x : y; }

// Basically distance between two numbers

// FLAG: This should really be signed. I know some buffer code uses it.
// Still, even size_t makes little sense.
static unsigned max_minus_min(unsigned int a, unsigned int b)
{
	return (unsigned)( a < b ? b - a : a - b );
}

static void convert_int(printfctl *pc)
{
	// It the output resulting from using printf/fprintf
	static const char pfx_tab[] = {0,'-','+','-',' ','-'};

	int N;

	char *pb = pc->buff + sizeof(pc->buff);

	{
		// FLAG: there is no need to clear the whole buffer when converting
		// an INT in practice. This can be down to a few moves instead.
		// You can also use iters, but that seems wasteful.
		// Is this even really necessary at all?
		// I know how many I generated.
		memset(pc->buff, '0', unsigned_decimal_digits[sizeof(uintmax_t)]);
		union xintmax xim;

		struct bcount bc = b2represent_from_fetch_iarg(&xim, pc, pc->v);
		uintmax_t a = xim.u;

		// FLAG:
		// Technically we need to ignore the minus sign, basically.
		// Consider that when doing this.
		N = !!(xim.i < 0);

		for (unsigned i = 0; i < bc.dec; i++)
		{
			pb--;
			*pb = (a % 10) + '0';
			a /= 10;
		}
	}

	// FLAG: Why not use memchr?

	// Find first non-zero character in the sequence
	// pb is at the last character generated, regardless of the type
	// so redundant iterations can be avoided.
	for (unsigned i = 0; i < sizeof(pc->buff); i++)
	{
		if (*pb != '0')
			break;
		pb++;
	}

	/*
	OUTPUT SECTION
	*/

	// FLAG: Why did I make this unsigned?
	// Why the cast?
	// It is a value we only perform basic operations with.
	unsigned chars_gen = (unsigned)((pc->buff + sizeof pc->buff) - pb);

	char pfxch = pfx_tab[pc->plus_sign*2+pc->prepend_space*4+N];

	if (chars_gen == 0) {
		chars_gen = 1;
	}

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
			// fputs("___ASDASD___", stderr);
			// getchar();
		}
		else {
			/*
			In this case, padding is necessary.
			*/
			// printf("\n(pr=%i, cg=%i)\n", pc->padding_req, chars_gen );
			// getchar();
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
			pc->precision > chars_gen ? pc->precision-chars_gen : 0;

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
			pc->cmt(pc, pb, (size_t)chars_gen);
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
__attribute__((noinline))
static void print_pad_nopfx_buff( // FLAG: chars_gen to int
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
			pc->precision > chars_gen ? pc->precision-chars_gen : 0;

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

// FLAG: TODO This
void convert_uint(printfctl *pc)
{
	// unsigned iters = uint_iters_tab[pc->length_mod];

	// char *pb = pc->buff + sizeof(pc->buff);

	// memset(pc->buff, '0', sizeof(pc->buff));

	// if (iters <= ndc_int) {
	// 	unsigned a = va_arg(*pc->v, unsigned);

	// 	for (unsigned i = 0; i < iters; i++) {
	// 		pb--;
	// 		*pb = (a % 10U) + '0';
	// 		a /= 10U;
	// 	}
	// }
	// else {
	// 	unsigned long long a = va_arg(*pc->v, unsigned long long);

	// 	for (unsigned i = 0; i < iters; i++) {
	// 		pb--;
	// 		*pb = (a % 10) + '0';
	// 		a /= 10;
	// 	}
	// }

	// for (unsigned i = 0; i < sizeof(pc->buff); i++) { /*-1?*/
	// 	if (*pb != '0')
	// 		break;
	// 	pb++;
	// }

	// size_t chars_gen = (unsigned)((pc->buff + sizeof pc->buff) - pb);

	// if (chars_gen == 0) chars_gen = 1;

	// print_pad_nopfx_buff(pc, chars_gen - 2, pb);
}

static unsigned itox(	uintmax_t	value,
						int			cap,
						int			maxdigits,
						char *		buff
){
	// Pack these together and align to cache boundary?

	static const char lookup1[16] =
	{'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
	static const char lookup2[16] =
	{'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

	const char * const lookup = cap ? lookup2 : lookup1;

	unsigned int nze = 0, ret = 0;

	for (int i = maxdigits-1; i >= 0; i--)
	{
		// FLAG: Can I get rid of this cast?
		// i will never be negative.
		char digit = lookup[ (value >> i*4) & 0xF ];

		if (!nze && digit != '0')
		nze=1;

		if (nze) {
			buff[ret] = digit;
			ret++;
		}
	}
	return ret;
}

// FLAG: This does NOT handle alternative # modifier.
// FLAG:
// Refactor the lookup tables into a convenient unified structure.
//
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

// Returns number of characters generated.
// Forward generating to the buffer.
// This is also always 64-bit because the cost is low.
static int itoo_fw(	unsigned long long	value,
					char *				buff
){
	unsigned int nonzero_encountered = 0;
	unsigned int ret = 0;

	for (int i = 21; i >= 0; i--)
	{
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

	// FLAG: needs to be restrict or not?
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

static const fmt_handler fmt_lookup[] = {
	['i'-'A'] = fmt_i,
	['u'-'A'] = fmt_u,
	['x'-'A'] = fmt_x,  // They are the same and autodetected internally
	['X'-'A'] = fmt_x,
	['s'-'A'] = fmt_s,
	['d'-'A'] = fmt_i,

	#if defined(SHARED_PRINTF_ENABLE_FLOAT)
	['g'-'A'] = fmt_g,
	['G'-'A'] = fmt_G,
	// ADD F when time comes.
	#endif

	['o'-'A'] = fmt_o,
	['p'-'A'] = fmt_p,
	['a'-'A'] = fmt_a,
	['A'-'A'] = fmt_A,
	['e'-'A'] = fmt_e,
	['E'-'A'] = fmt_E,
	['n'-'A'] = fmt_n
	// Notice no fmt_percent.
};

// FLAG: Signed char could lead to unexpected result.
// Highly unlikely in a string constant, but if it is negative it
// does little good to cast to unsigned, better to abs().
// Any character not in the basic execution character set can be negative.
// So if we try to print some CP437 character with printf this will actually
// crash!
//
// This is not necessarily true. Characters are not actually negative.
// They just get a different bit pattern that totally changes what it is.
// No such thing as -A or -B. That would change it completely.
// Just don't use it as an index.
//
static char is_valid_fmt(char f) // FLAG: Use int instead, no reason not to.
{
	// Only alphabetical character can be format characters.
	// % is not included (see set_fmt_params for why)

	if (isalpha(f) && fmt_lookup[(unsigned)f - (unsigned)'A'] != 0)
		return f;
	else
		return '\0';
}

static void set_fmt_params_defaults(printfctl *ctl)
{
	ctl->zero_default_flags = 0;

	// Integer that must be set manually
	ctl->padding_req = 0;

	// Justify is to the right by default!!!
	ctl->justify = 1;
}

// A format is like an instruction. This is the decode stage.
// the va_list is for the case in which we need use the * prefix.
static void set_fmt_params(printfctl * ctl, const char *f, unsigned inx)
{
	// The right-justified padding is placed at the beginning of the
	// format specifier. It does not need to be iteratively checked.
	// However, this HAS to be a loop despite the printf format being
	// structured quite well because the # modifier can be inserted
	// anywhere.
	// Maybe fact check this.

	// Local index for better optimization. Later copied to the ctl.
	unsigned int lx = ctl->inx;

	// FLAG: L is not required anymore for long double and l can be used too.
	// Per the standard.
	while (1)
	{
		// Is this a format specifier? If so, we are done.
		// Can be converted to a lookup as it is based on a char anyway.
		if ( (ctl->req_fmt = is_valid_fmt(f[lx])) != 0 )
		{
			lx++;
			break;
		}

		// Other things fit into a switch case.
		else switch (f[lx])
		{
			case 'h':
				// Probably wrong, refactor???????? Dont think so (May 20,2025)
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

			// FLAG: can you just increment the length mod at this point?
			// It is assued zero if the format is not malformed.
			// "hl" for example would never happen.
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
				// C99 requires lower case to work the same as L
				ctl->length_mod = l_l;
				lx++;
			break;

			case '%':
				// %% is a special case of a flag. No other flag should come
				// before it or the format is ill-formed, per GNU warnings.
				// For example, %10% is NOT legal and printf has no way of
				// knowing at any point in time that the second % is not a
				// second format.
				// %% is not implemented in a function, so our call table
				// remains base-A and small, and largely branchless.
				//
				ctl->dup(ctl, '%', 1);
				lx++;
				goto out;

			case '0':
				// Pad using leading zeroes.
				// This works even when another padding is
				// specified.
				// E.g. "%020f", 1.0 would print
				// 0000000000001.000000
				// Also works with integers.
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
					char *str_end;
					ctl->precision =
						strtoul(f + lx, &str_end, 10);
						lx = str_end - f + 1;
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
				if ( isdigit(f[lx]) ) {
					char *str_end;
					ctl->padding_req = strtoul(f + lx, &str_end, 10);
					lx = str_end - f + 1;
				}
		}
	}
	out:

	ctl->inx = lx;
}

int _printf_core(	printfctl *				ctl,
					char *  __restrict		buffer,
					commit_buffer_f			cmt,
					dupch_f					_dup,
					size_t					count,
					const char *__restrict	f,
					va_list					v
){
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

			// FLAG: This is wrong too
			fmt_lookup[(unsigned)ctl->req_fmt - (unsigned)'A'](ctl);
		}
		else {
			cmt(ctl, 0, (size_t)f[ctl->inx]);
			ctl->inx++;
		}
	}

	return (int)ctl->bytes_printed;
}

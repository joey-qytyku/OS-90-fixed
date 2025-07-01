/////////////////////////////////////////////////////////////////////////////
//                     Copyright (C) 2025, Joey Qytyku                     //
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

/*******************************************************************************

This printf uses the DJGPP implementation of printf as a reference
and is written to comply with the standard in ISO/IEC 9899:TC3.
It only works on the i386 ABI.

This implementation uses a fetch/decode/execute style with an internal state
to store decoded flags as operands.

The fprintf family which is implemented using functions here is limited to
4095 characters, which is allowed by the standard. snprintf and sprintf do not
have this limitation and can output to any large-enough buffer.

*******************************************************************************/

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <stdio.h>

// We do not use NULL because it is not part of the kernel headers.
#undef NULL

// This is used instead and is just zero.
#define Null 0

#define _assume(cond) {do {if (!(cond)) __builtin_unreachable();} while (0);}

#ifndef unlikely
	#define unlikely(x) __builtin_expect((x),0)
#endif

#ifndef likely
	#define likely(x) __builtin_expect((x),1)
#endif

typedef enum {
	l_NONE = 0,
	l_hh,
	l_h,
	// l_l,            // long
	// l_z,            // size_t
	// l_t,		// ptrdiff

	l_ll,           // long long
	l_j,            // intmax_t
	l_L		// Long float type
}lenmod;
/*
The buffer is an optimized representation of a character output distination.
Pointers are used to simplify the truncation byte count to `end` - `at`

`end` is NOT the last byte. This allows null snprintf to work with a null
destination because the size of the buffer is zero.
*/
typedef struct {
	char* start;
	char* end;
	char* at;
}PRINTF_BUFFER;

typedef struct {
	va_list v;
	PRINTF_BUFFER buff;

	char convert_buff[24];

	union {
		unsigned char zero_default_flags;
		struct {
			unsigned char alternate:1;
			// Add a space in the place of a minus sign.
			// Example: % i
			//
			unsigned char prepend_space:1;
			unsigned char plus_sign:1;
			unsigned char leading_zero_pad:1;
		};
	};
	lenmod length_mod;

	int padding_req;
	int precision;
	unsigned char justify:1; // True by default

	char req_fmt;
}PRINTF;

// 32-bit integer converter written in assembly
extern char* asm_v(unsigned int value, char* buffer);

static void WRITE(PRINTF_BUFFER* buff, const char *data, unsigned bytes)
{
	// The computed goto allows further calls to WRITE to do nothing
	// if the buffer is overrun, while updating buff->at such that
	// subtracting with buff->start gives the number of characters
	// originally requested in the case of trunctation with printf.
	static void* accept_or_reject_write = &&accept_write;

	_assume(buff != 0 && data != 0);

	accept_write:;

	// If the data fits
	if (buff->at + bytes < buff->end) {
		// Copy all of it
		memcpy(buff->at, data, bytes);
	}
	// If the data does NOT fit
	else {
		// Truncate
		memcpy(buff->at, buff->end - buff->at, data);
		buff->at += bytes;
		accept_or_reject_write = &&reject_write;
	}

	reject_write:;
}

static void DUP(PRINTF_BUFFER* buff, int ch, unsigned n)
{
	if (buff->at != buff->end)
	{
		*buff->at = (char)ch;
	}
	buff->at++;
}

static unsigned itox(	uint64_t	value,
			int		capitalize,
			int		maxdigits,
			char*		buff
)
{
	static const char lookup1[2][16] =
	{
		{'0','1','2','3','4','5','6','7',
		 '8','9','a','b','c','d','e','f'},
		{'0','1','2','3','4','5','6','7',
		 '8','9','A','B','C','D','E','F'}
	};

	const char* lookup = lookup1[capitalize];

	unsigned int nze = 0, ret = 0;

	for (int i = maxdigits-1; i >= 0; i--)
	{
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

static int itoo(uint64_t value, char* buff)
{
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

static void get_scalar(PRINTF* pc, void* to)
{
	if (pc->length_mod >= l_ll)
	{
		long long value = va_arg(pc->v, long long);
		memcpy(to, (void*)&value, sizeof(long long));
	}
	else
	{
		int value = va_arg(pc->v, int);
		memcpy(to, (void*)&value, sizeof(int));
	}
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
// NOTE: This probably works, it just does not fetch arguments
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

static void set_fmt_params_defaults(PRINTF* ctl)
{
	ctl->zero_default_flags = 0;

	// Integer that must be set manually
	ctl->padding_req = 0;

	// Justify is to the right by default!!!
	ctl->justify = 1;
}

// Allows the dispatch table to be accessible despite its static scope.
static const void ** exec_dispatch_table_alias = 0;

static void execute(PRINTF* ctl)
{
	// FLAG: This cannot be moved outside, unfortunately.
	// Making it static does allow another variable to reference it.
	static void* table[] =
	{
		['i'-'A'] = &&fmt_i,
		['u'-'A'] = &&fmt_u,
		['x'-'A'] = &&fmt_x,
		['X'-'A'] = &&fmt_X,
		['s'-'A'] = &&fmt_s,
		['d'-'A'] = &&fmt_i,

		#if defined(SHARED_PRINTF_ENABLE_FLOAT)
		['g'-'A'] = &&fmt_g,
		['G'-'A'] = &&fmt_G,
		// ADD F when time comes.
		#endif

		['o'-'A'] = &&fmt_o,
		['p'-'A'] = &&fmt_p,
		['a'-'A'] = &&fmt_a,
		['A'-'A'] = &&fmt_A,
		['e'-'A'] = &&fmt_e,
		['E'-'A'] = &&fmt_E,
		['n'-'A'] = &&fmt_n
	};
	exec_dispatch_table_alias = table;

	goto *(table[ctl->req_fmt - 'A']);

	fmt_i:
		format_signed_int(ctl);
		return;
	fmt_u:
		format_unsigned_int(ctl);
		return;
	fmt_X:
		int capitalize = 1;
	fmt_x: {
		unsigned long long value = 0;
		get_scalar(ctl, &value);

		return;
	}
	fmt_s: {
		;
	}
	fmt_g: {
		;
	}
	fmt_G: {
		;
	}
	fmt_o: {
		;
	}
	fmt_p: {
		;
	}
	fmt_a: {
		;
	}
	fmt_A: {
		;
	}
	fmt_e: {
		;
	}
	fmt_E: {
		;
	}
	fmt_n: {
		;
	}
}

static char is_valid_fmt(int f)
{
	return isalpha(f) && exec_dispatch_table_alias[f-'A'] != 0 ? f : '\0';
}

static void decode_flags(PRINTF* ctl, const char *f, unsigned _lx)
{
	unsigned lx = _lx;
	// The right-justified padding is placed at the beginning of the
	// format specifier. It does not need to be iteratively checked.
	// However, this HAS to be a loop because the # modifier can be
	// inserted anywhere.

	while (1)
	{
		if ( (ctl->req_fmt = is_valid_fmt(f[lx])) != 0 )
		{
			lx++;
			break;
		}
		else switch (f[lx])
		{
			case 'h':
				if (f[lx+1]=='h') {
					ctl->length_mod = l_hh;
					lx+=2;
				}
				else {
					ctl->length_mod = l_h;
					lx++;
				}
			break;

			// FLAG: can you just increment the length mod at this point?
			// It is assued zero if the format is not malformed.
			// "hl" for example would never happen.
			case 'l':
				// If we already encountered 'l' before
				// set the mod to ll.
				// This is safe because we already are in
				// a format and f[lx-1] may just be a % sign.
				if (f[lx-1] == 'l')
				{
					ctl->length_mod = l_ll;
				}
			break;

			case 'j': // intmax_t
				ctl->length_mod = l_ll;
				lx++;
			break;

			case 'z': // size_t
				ctl->length_mod = l_NONE;
				lx++;
			break;

			case 't': // ptrdiff_t
				ctl->length_mod = l_NONE; // ?????
				lx++;
			break;

			case 'L': // long double
				ctl->length_mod = l_L;
				lx++;
			break;

			case '%':
				// %% is a special case of a flag.
				// The standard requires no other flags to be
				// valid in this case.
				DUP(&ctl->buff, '%', 1);
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
				if (f[lx] == '*')
				{
					// FLAG:
					// This is undefined behavior.
					// Probably not a big deal on x86.
					ctl->precision = va_arg(ctl->v, unsigned);
					lx++;
				}
				else if (f[lx] >= '1' && f[lx] <= '9')
				{
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

				ctl->padding_req = va_arg(ctl->v, unsigned);
				lx++;
			break;

			default:
				if ( isdigit(f[lx]) )
				{
					char *str_end;
					ctl->padding_req = strtoul(
								f + lx,
								&str_end, 10
							);
					lx = str_end - f + 1;
				}
		}
	}
	out:;
}

int _vsnprintf(	char* restrict		buffer,
		unsigned		bufsz,
		const char *restrict	format,
		va_list			vlist)
{
	PRINTF p;
	unsigned i = 0;

	while (i = 0, format[i] != '\0')
	{
		if (format[i] == '%')
		{
		}
		i++;
	}
}

int _snprintf(char *restrict buffer, size_t size, const char * restrict fmt)
{
}

static void test_printf(const char *f, size_t sz, ...)
{
	va_list v, a1, a2;
	static char b1[4096];
	static char b2[4096];

	b1[4095] = 0;
	b2[4095] = 0;

	va_start(v, f);

	va_copy(a1, v);
	va_copy(a2, v);

	int n = _vsnprintf(f, b1, sz, a1);
	int m = vsnprintf (f, b2, sz, a2);


	if (n != m || strlen(b1) != strlen(b2) || memcmp(b1, b2, n) != 0)
	{
		printf( "\n     Test Failed\n"
			"-----------------------\n"
			"standard:\n%s\n"
			"custom  :\n%s\n",
			b1,
			b2
		);
	}

	va_end(v);
}

static void run_integer_tests()
{
}

int main()
{
}

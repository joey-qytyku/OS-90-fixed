/*******************************************************************************

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

*******************************************************************************/

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

// We do not use NULL here. The kernel headers do not define it by default
// and we just use zero to explicitly state that a pointer is set to zero.
#undef NULL

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
Pointers are used to simplify the truncation byte count to end - at
*/
typedef struct {
	char* start;
	char* end;
	char* at;
}PRINTF_BUFFER;

typedef struct {
	va_arg v;
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

	unsigned char justify:1; // True by default

	char req_fmt;
}PRINTF;

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
	if (buff->at + bytes <= end) {
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
		long long value = va_arg(*pc->v, long long);
		memcpy(to, (void*)&value, sizeof(long long));
	}
	else
	{
		int value = va_arg(*pc->v, int);
		memcpy(to, (void*)&value, sizeof(int));
	}
}

static void format_str_nopfx(	PRINTF*		pc,
				const char*	pb,
				size_t		chars_gen)
{}

// FLAG: Maybe it should return something?
// Why though?
static void format_signed_int(PRINTF*)
{}

static void set_fmt_params_defaults(printfctl *ctl)
{
	ctl->zero_default_flags = 0;

	// Integer that must be set manually
	ctl->padding_req = 0;

	// Justify is to the right by default!!!
	ctl->justify = 1;
}

// Allows the dispatch table to be accessible despite its static scope.
static void **exec_dispatch_table_alias = 0;

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
				DUP(ctl->buff, '%', 1);
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

int _vsnprintf(		char* restrict		buffer,
			unsigned		bufsz,
			const char *restrict	format
			va_list			vlist)
{
	unsigned i = 0;
	while (i = 0, format[i] != '\0')
	{
		if (format[i] == '%') {
			decode_flags(format, i+1);
		}
		i++;
	}
}

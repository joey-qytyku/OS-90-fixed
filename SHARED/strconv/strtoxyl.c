#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

union out {
	unsigned long long u64;
	long long i64;
};

// Takes lower case or upper case number and returns the integer representation
// of the digit. Multiplied by the base to the power of the current digit
// and summed to accumulator when converting.
//
// This was auto-generated.

static unsigned digit_char_num_lut[] = {
	['0'-'0'] = 0,
	['1'-'0'] = 1,
	['2'-'0'] = 2,
	['3'-'0'] = 3,
	['4'-'0'] = 4,
	['5'-'0'] = 5,
	['6'-'0'] = 6,
	['7'-'0'] = 7,
	['8'-'0'] = 8,
	['9'-'0'] = 9,
	['a'-'0'] = 10,
	['b'-'0'] = 11,
	['c'-'0'] = 12,
	['d'-'0'] = 13,
	['e'-'0'] = 14,
	['f'-'0'] = 15,
	['g'-'0'] = 16,
	['h'-'0'] = 17,
	['i'-'0'] = 18,
	['j'-'0'] = 19,
	['k'-'0'] = 20,
	['l'-'0'] = 21,
	['m'-'0'] = 22,
	['n'-'0'] = 23,
	['o'-'0'] = 24,
	['p'-'0'] = 25,
	['q'-'0'] = 26,
	['r'-'0'] = 27,
	['s'-'0'] = 28,
	['t'-'0'] = 29,
	['u'-'0'] = 30,
	['v'-'0'] = 31,
	['w'-'0'] = 32,
	['x'-'0'] = 33,
	['y'-'0'] = 34,
	['z'-'0'] = 35,
	['A'-'0'] = 10,
	['B'-'0'] = 11,
	['C'-'0'] = 12,
	['D'-'0'] = 13,
	['E'-'0'] = 14,
	['F'-'0'] = 15,
	['G'-'0'] = 16,
	['H'-'0'] = 17,
	['I'-'0'] = 18,
	['J'-'0'] = 19,
	['K'-'0'] = 20,
	['L'-'0'] = 21,
	['M'-'0'] = 22,
	['N'-'0'] = 23,
	['O'-'0'] = 24,
	['P'-'0'] = 25,
	['Q'-'0'] = 26,
	['R'-'0'] = 27,
	['S'-'0'] = 28,
	['T'-'0'] = 29,
	['U'-'0'] = 30,
	['V'-'0'] = 31,
	['W'-'0'] = 32,
	['X'-'0'] = 33,
	['Y'-'0'] = 34,
	['Z'-'0'] = 35
};

/*
This implementation is quite horrible, but the standard is mostly to blame.

NOTE: THIS MODIFIES ERRNO, MAY NEED TO CHANGE

This converts a 64-bit value (long long).
*/
static
union out
_strtoxll(
	int is_signed,
	const char * restrict str,
	char ** restrict end_str,
	int base
)
{
	// ADD ERNO CONDITIONS, including base if 1

	int do_unary_minus = 0;

	// Find the first non-space character.
	for (; *str != '\0' && isspace(*str); str++);

	// If a nul is found right away, return zero.
	if (str[0] == '\0') {
		union out out;
		out.u64 = 0;
		return out;
	}

	// If base is 0, we have to autodetect.
	// If the prefix is 0x or 0X, then it is assumed to be a hex.
	// If there is only a 0, it is octal.
	// Otherwise, the base is decimal.
	// No other base can be detected.

	if (base == 0) {
		if (str[0] == '0') {
			if (isdigit(str[1])) {
				base = 8;
			}
			else if (str[1] == 'x' || str[1] == 'X') {
				base = 16;
				str+=2;
			}
			else if (str[1] == '\0') {
				// Output is zero!!<<<<<
			}
		}
		else {
			// Otherwise we started with a regular integer character
			// and will assume base 10.
			base = 10;
		}
	}

	if (*str == '-') {
		do_unary_minus = 1;
		str++;
	}

	// At this point `str` is pointing to the first character to start
	// reading and the base is set.

	char *p = str;

	// While the character is alphanumeric and thus a possibly valid
	// digit AND it is also in range
	while (isalnum(*p) && digit_char_num_lut[toupper(*p)-'0'] < base)
	{
		if ((p - str) == 19) {
			errno = ERANGE;
			union out r;
			r.i64 = -1LL;
			return r;
		}
		p++;
	}

	if (end_str != NULL)
		*end_str = str+1;

	unsigned long long acc = 0LL;
	unsigned long long mul = 1;
	union out out;

	while (p > str) {
		p--;
		acc += mul * digit_char_num_lut[*p-'0'];
		mul *= base;
	}

	if (do_unary_minus) {
		out.u64 = -acc;
	}
	else {
		out.u64 = acc;
	}

	return out;
}

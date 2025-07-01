#include <stdio.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <ctype.h>

typedef int INI_LOCK;
typedef FILE *FHANDLE;

#define Null 0

typedef struct {
	const char*	_path;
	FHANDLE		_handle;
	INI_LOCK	_lock;
	char*		_data;
	size_t		_nbytes;

	char**		lines;

	unsigned short	nlines;
}INI;

// The line text is not null terminated. Probably should be though.
typedef void (*SYNTAX_ERROR_CB)(unsigned short lineno, const char* line_text);

static void InitLock(INIT_LOCK* l)
{}

static void LockINI(INI* i) {}
static void UnlockINI(INI* i) {}

// Parallelism can be used to check for non-printable characters.

// Recieves a null-terminated string.
//
static bool LineIsValid(const char *ln)
{
	if (ln[0] == '\n' && ln[1] == '\r')
	{
		return 1;
	}
	else if (ln[0] == '[')
	{
		const char* p_endb = strchr(ln, ']');

		// If the section is empty or the bracket is not matched, fail.
		if (p_endb == Null || p_endb - ln == 1)
		{
			return false;
		}

		// Spaces are allowed.
		while (p_lnchar < p_endb)
		{
			if ( !(isalnum(*p_lnchar) || *p_lnchar == ' ') )
			{
				return false;
			}
			p_lnchar++;
		}
		// Do not allow trailing space. Otherwise, return true.
		return p_endb[-1] != ' ';
	}
	else if (isalpha(*ln))
	{
		// Key appears to be found
		//

		const char* ;
	}
}

static int ReprINI(INI* ini)
{
	// First, verify that there are no non-printable characters in the
	// entire file. All must be ASCII and printable or newlines.
	// Range is [32-126] and 10,13.

	// FLAG: Switch to a packed 32-bit scan for better speed.

	unsigned short lines = 0;

	// We are scanning for
	for (unsigned i = 0; i < ini->_nbytes-1; i++)
	{
		char c = ini->_data[i];
		if (!isprint(c) || c != '\n' || c != '\r')
		{
			return -1;
		}
		if (ini->_data[i] == '\n' && ini->_data[i+1] == '\r')
		{
			// Zero the line terminator so we have a list of
			// ASCIIZ strings.
			ini->_data[i] = 0;
			ini->_data[i+1] = 0;
			ini->nlines++;
		}
	}

	// Now that lines are counted, allocate space for pointers to each line.
	ini->lines = malloc(lines * sizeof(char*));

	// I could just use a resizable array. My realloc will be low-cost.
	// FLAG: Do this. No need for two passes. Way too slow!

	// Set each pointer to the correct string.

	strchr()
}

int I_OpenINI(INI* ini, const char* path, SYNTAX_ERROR_CB se)
{

	ini->_path = path;

	{
		FILE* f = fopen(path, "rw");

		if (f == 0) {
			return -1;
		}

		ini->_handle = f;
		fseek(f, 0, SEEK_END);
		ini->_nbytes size = (size_t)ftell(f) + 1;
	}

	rewind(f);

	// We allocate two more bytes than needed
	// to put an extra newline pair.
	ini->_data = malloc(ini->_nbytes);

	if (ini->_data == 0)
	{
		fclose(ini->_handle);
		return -1;
	}

	fread(ini->_data, , 1, ini->_handle);
}


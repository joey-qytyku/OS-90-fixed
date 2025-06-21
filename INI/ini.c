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
		// Check if the characters between are alphanumeric.
		// The first must be alphabetical.

		const char *p_lnchar = ln+1;
		if (!isalpha(*p_lnchar))
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

int I_OpenINI(INI* ini, const char* path, SYNTAX_ERROR_CB)
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


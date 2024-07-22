#include <stdio.h>
#include <stdint.h>

typedef struct {
	BYTE    name_chars[20];
}SOBJ, PSOBJ;

typedef struct {

}STRPARSE_STATE;

typedef struct {
	uintptr_t       regs[8];
	char*           code;

	uint32_t        line;
	uint32_t        line_off;

	uint32_t        function_level;
	uintptr_t       stk[64];
	uint32_t        sp;
}STATE, PSTATE;

// I need functions for eating integers, variables, regs, etc.
// They should report how many characters were eaten.
// For example, to detect if an array element was not initialized.

// Returns NUL if reached end of line or semicolon.
static char EatNextChar(PSTATE ps);

// Read next integer, Can be hex if prefixed with
static uint32_t EatNextInt(PSTATE ps);
static void EatNextString(PSTATE ps, char *buff, size_t buff_len)

//
// 0: Did not find a symbol, string, expression, or integer
// 1: INT
// 2: Expression block
// 3: String literal
// 4: Symbol name
//
static int EatNextSymStringExrInt(PSTATE ps, char **buff, size_t buff_len);

static void ExecuteScript(SCRIPT_STATE *st);

// static void EatNext

int main(int argc, char **argv)
{
	return 0;
}

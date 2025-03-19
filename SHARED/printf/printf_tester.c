#define SHARED_PRINTF_TESTING_NATIVE
#define SHARED_PRINTF_LLONG 1

#include "printf.c"

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

static void _putchar_dup(	printfctl *	ctl,
				char		c,
				size_t		count
){
	// if (count > ctl->bytes_left) {
	// 	count = ctl->bytes_left;
	// }
	ctl->bytes_printed += count;

	for (size_t i = 0; i < count; i++)
		fputc(c, stderr);
}

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
                size_t actual_size = b == NULL ? 1 : umin(c, ctl->bytes_left);

                memcpy(ctl->out_buffer, b, actual_size);
                ctl->out_buffer += actual_size;
                ctl->bytes_printed += actual_size;
                ctl->bytes_left -= actual_size;
        }
}

void _byte_buff_dup(printfctl *pc, char c, size_t count)
{

}

int _printf(const char *__restrict f, ...)
{
	va_list args;
	va_start(args, f);

	printfctl ctl;

	int r = _printf_core(
		&ctl,
		NULL,
		_putchar_commit,
		_putchar_dup,
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

	int r = _printf_core(&ctl,s,_byte_buff_commit,_byte_buff_dup,n,f,args);

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

//

int main(void)
{
	#define TEST "[%40.4s]\n", "Hello world!"
	_printf(TEST);
	fprintf(stderr, TEST);
}
// ACTUALLY NOT QUITE. Passing a number that is smaller than an int is an
// implicit promotion, which is a special rule with C.
//
// If char is unsigned it will actually sign extend.
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

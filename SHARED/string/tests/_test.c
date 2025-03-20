#include <stdio.h>
#include <string.h>

// extern char *my_strchr(const char *str, int ch);
int my_memcmp(const void*, const void*, unsigned);

const char a[] = "Hello";
const char b[] = "Hello";

int main()
{
	printf("%p", my_memcmp(a, b, 4));
	return 0;
}

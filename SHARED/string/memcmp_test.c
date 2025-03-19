#include <stdio.h>
#include <string.h>

char a[128] = {1,2,3,2,10,2,10,15};
char b[128] = {1,2,3,4,9, 8,7,6};

extern int my_memcmp(const void *l, const void *r, unsigned n);

int main()
{
	printf("%i\n", memcmp(a, b, 0) );
	printf("%i\n", my_memcmp(a, b, 0) );
}

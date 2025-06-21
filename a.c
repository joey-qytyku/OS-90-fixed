#include <stdio.h>
#include <stdint.h>

int main()
{
	for (int i = 0; i < 100000; i++)
	{
		if (0xFF * i == 0xF0F0) {
			printf("FOUND: %i", i);
		}
	}
	puts("Done");
}

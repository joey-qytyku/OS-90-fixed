#include <stdio.h>

int main()
{
	for (int i = 0; i <= 9; i++) {
		printf("['%i'-'0'] = %i,\n", i,i);
	}
	for (int j = 0; j < 26; j++) {
		printf("['%c'-'0'] = %i,\n", 'a'+j, j+10);
	}
	for (int j = 0; j < 26; j++) {
		printf("['%c'-'0'] = %i,\n", 'A'+j, j+10);
	}
}

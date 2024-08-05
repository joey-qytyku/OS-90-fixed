// Code is public domain.
// Compile with IA-16 GCC

#include <i86.h>
#include <dos.h>

int main()
{
	puts(
		"Create swap file utility; public domain\n"
		"---------------------------------------"
	);

	{
		puts("Are you on your boot drive? [y|n]:");
		if (getchar() != 'y') {
			puts("The swap file must be on the boot drive.\n"
			"Please enter the letter:")
			setdisk(getchar());
		}
	}

	long multiple = 0;

	{
		puts("In what quantity do you wish to allocate the swap file? [k|m]");
		char c = getchar();
		if (c == 'k' || c == 'K')
			multiple = 1024L;
		else if (c == 'm' || c == 'M')
			multiple = 1024L*1024L;
	}

	puts("How many do you want? [number]");
	long want = 0L;
	scanf("%i", &want);
	if (want == 0) {
		puts("Invalid input.");
		return 1;
	}
	want *= multiple;

	puts("Creating swap file");

	// TODO
}

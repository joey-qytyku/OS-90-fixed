#include <stdio.h>

int main()
{
	unsigned short ebda_seg = *((unsigned short*)(0x40E));

	printf("EBDA segment %x\n", ebda_seg);
	printf("EBDA is %u K bytes\n", *((unsigned short*)(ebda_seg<<4)));
}

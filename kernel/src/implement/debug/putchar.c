#include "misc/io.h"

void _putchar(char c)
{
    outb(0xE9, c);
}

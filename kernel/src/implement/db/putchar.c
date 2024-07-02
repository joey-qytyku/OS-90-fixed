#include <osk/mc/pio.h>

void _putchar(char c)
{
    outb(0xE9, c);
}

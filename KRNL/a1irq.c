/////////////////////////////////////////////////////////////////////////////
//                     Copyright (C) 2022-2025, Joey Qytyku                //
//                                                                         //
// This file is part of OS/90.                                             //
//                                                                         //
// OS/90 is free software. You may distribute and/or modify it under       //
// the terms of the GNU General Public License as published by the         //
// Free Software Foundation, either version two of the license or a later  //
// version if you chose.                                                   //
//                                                                         //
// A copy of this license should be included with OS/90.                   //
// If not, it can be found at <https://www.gnu.org/licenses/>              //
/////////////////////////////////////////////////////////////////////////////

static const unsigned char IRQ_BASE        = (0xA0);
static const unsigned char ICW1            = (1<<4);
static const unsigned char LEVEL_TRIGGER   = (1<<3);
static const unsigned char ICW1_ICW4       = 1;
static const unsigned char ICW4_8086       = 1;
static const unsigned char ICW4_SLAVE      = 1<<3;

void API SetIrqMask(short m)
{
	delay_outb(0x21, m & 0xFF);
	delay_outb(0xA1, m >> 8);
}

short API GetIrqMask(void)
{
	return delay_inb(0x21) | (delay_inb(0xA1)<<8);
}

// This is done literally only once. Just do this as a static inline.
void RemapPIC(void)
{
	const short m = GetIrqMask();

	// ICW1 to both PIC's

	delay_outb(0x20, ICW1 | ICW1_ICW4);
	delay_outb(0xA0, ICW1 | ICW1_ICW4);

	// Set base interrupt vectors
	delay_outb(0x21, IRQ_BASE);
	delay_outb(0xA1, IRQ_BASE+8);

	// ICW3, set cascade
	delay_outb(0x21, 4); // For master it is a bit mask
	delay_outb(0xA1, 2); // For slave it is an INDEX

	// Send ICW4
	delay_outb(0x21, ICW4_8086);
	delay_outb(0xA1, ICW4_8086 | ICW4_SLAVE); // Assert PIC2 is slave

	// Rewrite mask register
	SetIrqMask(m);
}


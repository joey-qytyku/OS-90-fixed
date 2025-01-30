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

#include "l_segmnt.h"

extern SEGMENT_DESCRIPTOR gdt[], ldt[];

#define GET_DESC(selector) \
	( ( ((selector)>>2) & 1 ? ldt : gdt) + ( (selector) >> 3) )

VOID API L_SegmentSetBase(      SHORT   selector,
				LONG    base_addr)
{
	SEGMENT_DESCRIPTOR *s = GET_DESC(selector);

	s->s_base1 = base_addr & 0xFFFF;
	s->b_base2 = (base_addr>>16) & 0xFF;
	s->b_base3 = (base_addr>>24) & 0xFF;
}

VOID API L_SegmentSetLimit(     SHORT   selector,
				LONG    limit)
{
	SEGMENT_DESCRIPTOR
		*s = GET_DESC(selector);

	s->limit1 = limit & 0xFFFF;
	s->extaccess_byte |= (limit >> 16) & 0xF;
}

LONG API L_SegmentGetLimit(SHORT selector)
{
	SEGMENT_DESCRIPTOR
		*s = GET_DESC(selector);

	return s->limit1 | ((s->extaccess_byte & 0xF)<<16);
}

VOID API L_SegmentCreate(       SHORT   selector,
				LONG    base_addr,
				LONG    limit,
	                        LONG    access,
				LONG    exaccess)
{
	SEGMENT_DESCRIPTOR *s = GET_DESC(selector);

	s->extaccess_byte = exaccess;
	s->access_byte = access;

	L_SegmentSetLimit(selector, limit);
	L_SegmentSetBase(selector, base_addr);
}

#include "L_SEGMNT.H"

extern SEGMENT_DESCRIPTOR gdt[], ldt[];

// NOTE: 40h in GDT must be reserved for Windows
// to be used for BIOS data area or something.

#define GET_DESC(selector) \
	(( (selector) & 4 == 0 ? gdt : ldt) + ( (selector) >> 3))

VOID API L_SegmentSetBase(SHORT selector, LONG base_addr)
{
	SEGMENT_DESCRIPTOR *s = gdt+(selector>>3);//GET_DESC(selector);
	s->s_base1 = base_addr & 0xFFFF;
	s->b_base2 = (base_addr>>16) & 0xFF;
	s->b_base3 = (base_addr>>24) & 0xFF;
}

// Will not clobber the high portion of extended access
VOID API L_SegmentSetLimit(SHORT selector, LONG limit)
{
	SEGMENT_DESCRIPTOR *s = gdt+(selector>>3);//GET_DESC(selector);
	s->limit1 = limit & 0xFFFF;
	s->extaccess_byte |= (limit >> 16) & 0xF;
}

LONG API L_SegmentGetLimit(SHORT selector);

VOID API L_SegmentCreate(
	SHORT   selector,
	LONG    base_addr,
	LONG    limit,
	LONG    access,
	LONG    exaccess)
{
	SEGMENT_DESCRIPTOR *s = gdt+(selector>>3);//GET_DESC(selector);

	s->extaccess_byte = exaccess;
	s->access_byte = access;

	L_SegmentSetLimit(selector, limit);
	L_SegmentSetBase(selector, base_addr);
}

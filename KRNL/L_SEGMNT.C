#include "L_SEGMNT.H"

// NOTE: 40h in GDT must be reserved for Windows
// to be used for BIOS data area or something.

SEGMENT_DESCRIPTOR gdt[8];
SEGMENT_DESCRIPTOR ldt[128];

#define GET_DESC(selector) \
	(( (selector) & 4 == 0 ? gdt : ldt) + ( (selector) >> 3))

VOID API L_SegmentSetBase(SHORT selector, LONG base_addr)
{
	SEGMENT_DESCRIPTOR *s = GET_DESC(selector);
	s->s_base1 = base_addr & 0xFFFF;
	s->b_base2 = (base_addr>>16) & 0xFF;
	s->b_base3 = (base_addr>>24) & 0xFF;
}

// Will not clobber the high portion of extended access
VOID API L_SegmentSetLimit(SHORT selector, LONG limit)
{
	SEGMENT_DESCRIPTOR *s = GET_DESC(selector);
	s->limit1 = limit & 0xFFFF;
	s->extended.limit2 = (limit >> 16) & 0xF;
}

LONG API L_SegmentGetLimit(SHORT selector);

VOID API L_SegmentCreate(
	SHORT   selector,
	LONG    base_addr,
	LONG    limit,
	BYTE    access,
	BYTE    exaccess)
{
	SEGMENT_DESCRIPTOR *s = GET_DESC(selector);

	s->access.byte = access;
	s->extended.byte |= exaccess;

	L_SegmentSetLimit(selector, limit);
	L_SegmentSetBase(selector, base_addr);
}

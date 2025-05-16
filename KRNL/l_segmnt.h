/*******************************************************************************
		      Copyright (C) 2022-2025, Joey Qytyku

  This file is part of OS/90.

  OS/90 is free software. You may distribute and/or modify it under
  the terms of the GNU General Public License as published by the
  Free Software Foundation, either version two of the license or a later
  version if you choose.

  A copy of this license should be included with OS/90.
  If not, it can be found at <https://www.gnu.org/licenses/>
*******************************************************************************/

#ifndef DESC_H
#define DESC_H

typedef struct __attribute__((packed)) {

	unsigned short	limit1;
	unsigned short	s_base1;
	unsigned char	b_base2;

	unsigned char	access_byte;
	unsigned char	extaccess_byte;
	unsigned char	b_base3;

}SEGMENT_DESCRIPTOR;

API_DECL(void,  L_SegmentCreate,
	unsigned short	selector,
	unsigned	base_addr,
	unsigned	limit,
	unsigned char	access,
	unsigned char	exaccess
);

API_DECL(void,  L_SegmentSetLimit,
		unsigned short   selector,
		unsigned  limit
);


#endif /* DESC_H */

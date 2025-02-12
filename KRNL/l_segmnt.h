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

	WORD	limit1;
	WORD	s_base1;
	BYTE    b_base2;

	BYTE    access_byte;
	BYTE    extaccess_byte;
	BYTE    b_base3;

}SEGMENT_DESCRIPTOR;

API_DECL(VOID,  L_SegmentCreate,
		WORD	selector,
		DWORD	base_addr,
		DWORD	limit,
		DWORD	access,
		DWORD	exaccess
		);

API_DECL(VOID,  L_SegmentSetLimit,
		WORD   selector,
		DWORD  limit
		);


#endif /* DESC_H */

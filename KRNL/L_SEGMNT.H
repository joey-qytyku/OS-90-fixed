/////////////////////////////////////////////////////////////////////////////
//                     Copyright (C) 2022-2024, Joey Qytyku                //
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

#ifndef DESC_H
#define DESC_H

typedef struct __attribute__((packed)) {

	SHORT   limit1;
	SHORT   s_base1;
	BYTE    b_base2;

	BYTE    access_byte;
	BYTE    extaccess_byte;
	BYTE    b_base3;

}SEGMENT_DESCRIPTOR,
*PSEGMENT_DESCRIPTOR;

API_DECL(VOID,  L_SegmentCreate,
		SHORT   selector,
		LONG    base_addr,
		LONG    limit,
		LONG    access,
		LONG    exaccess
		);

API_DECL(VOID,  L_SegmentSetLimit,
		SHORT   selector,
		LONG    limit
		);


#endif /* DESC_H */

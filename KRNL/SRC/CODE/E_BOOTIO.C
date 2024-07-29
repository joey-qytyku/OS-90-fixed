/*******************************************************************************
		      Copyright (C) 2022-2024, Joey Qytyku

  This file is part of OS/90.

  OS/90 is free software. You may distribute and/or modify it under
  the terms of the GNU General Public License as published by the
  Free Software Foundation, either version two of the license or a later
  version if you choose.

  A copy of this license should be included with OS/90.
  If not, it can be found at <https://www.gnu.org/licenses/>
*******************************************************************************/

#include <Type.h>

#include <OSK/SD/stdregs.h>
#include <OSK/SD/sv86.h>

// Any non-alphanumeric byte is incorrect.
BYTE EarlyGetChar(BOOL req_enter)
{
	STDREGS r = V86R_INIT;
	r.AH = 0;
	V_INTxH(0x16, &r);
	return r.AL;
}

VOID EarlyGetString(BYTE *buff, SHORT size)
{
	STDREGS r = V86R_INIT;
	r.AH = 0;

	SHORT buffpos = 0;

	while (1)
	{
		V_INTxH(0x16, &r);

		if (r.AX == 0x1C0D) // Key == ENTER
			break;

		buff[buffpos] = r.AL;
		buffpos++;
	}
}

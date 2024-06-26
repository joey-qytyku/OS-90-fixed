/*
  浜様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様融
  �                  Copyright (C) 2023-2028, Joey Qytyku                    �
  麺様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様郵
  � This file is part of OS/90 and is published under the GNU General Public �
  �   License version 2. A copy of this license should be included with the  �
  �     source code and can be found at <https://www.gnu.org/licenses/>.     �
  藩様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様夕
*/

#include <Type.h>

#include <osk/sd/stdregs.h>
#include <osk/sd/sv86.h>

// Any non-alphanumeric byte is incorrect.
BYTE EarlyGetChar(BOOL req_enter)
{
        STDREGS r;
        r.AH = 0;
        OS_INTxH_t12(0x16, &r);
        return r.AL;
}

VOID EarlyGetString(BYTE *buff, SHORT size)
{
        STDREGS r;
        r.AH = 0;

        SHORT buffpos = 0;

        while (1)
        {
                OS_INTxH_t12(0x16, &r);

                if (r.AX == 0x1C0D) // Key == ENTER
                        break;

                buff[buffpos] = r.AL;
                buffpos++;
        }
}

/*
  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
  บ                  Copyright (C) 2023-2028, Joey Qytyku                    บ
  ฬออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออน
  บ This file is part of OS/90 and is published under the GNU General Public บ
  บ   License version 2. A copy of this license should be included with the  บ
  บ     source code and can be found at <https://www.gnu.org/licenses/>.     บ
  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

#include <Type.h>

#include <osk/sd/stdregs.h>
#include <osk/sd/sv86.h>

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

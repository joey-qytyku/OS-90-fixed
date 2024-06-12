/*
  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
  บ                  Copyright (C) 2023-2024, Joey Qytyku                    บ
  ฬออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออน
  บ This file is part of OS/90 and is published under the GNU General Public บ
  บ   License version 2. A copy of this license should be included with the  บ
  บ     source code and can be found at <https://www.gnu.org/licenses/>.     บ
  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/
#include <osk/sd/basicatomic.h>
#include <osk/sd/stdregs.h>
#include <osk/sd/sv86.h>
#include <osk/sd/int.h>

#include <osk/db/debug.h>

extern VOID M_Init(VOID);

STDREGS r;

VOID KernelMain(VOID)
{
        printf("Hello\n");
        M_Init();

        STI();

        // r.AH = 0xE;
        // r.AL = 'A';
        // r.BX = 0;
        // V_INTxH(0x10, &r);
}

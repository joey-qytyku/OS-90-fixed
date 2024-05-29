/*
  浜様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様融
  �                  Copyright (C) 2023-2024, Joey Qytyku                    �
  麺様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様郵
  � This file is part of OS/90 and is published under the GNU General Public �
  �   License version 2. A copy of this license should be included with the  �
  �     source code and can be found at <https://www.gnu.org/licenses/>.     �
  藩様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様夕
*/
#include <osk/sd/basicatomic.h>
#include <osk/sd/stdregs.h>
#include <osk/sd/sv86.h>
#include <osk/sd/int.h>

#include <osk/db/debug.h>

VOID KernelMain(VOID)
{
        printf("Hello\n");
        STI();
}

/*
  ��������������������������������������������������������������������������ͻ
  �                  Copyright (C) 2023-2028, Joey Qytyku                    �
  ��������������������������������������������������������������������������͹
  � This file is part of OS/90 and is published under the GNU General Public �
  �   License version 2. A copy of this license should be included with the  �
  �     source code and can be found at <https://www.gnu.org/licenses/>.     �
  ��������������������������������������������������������������������������ͼ
*/

#ifndef I386_H
#define I386_H

STACK_PARAMS PVOID i386GetDescriptorAddress(PVOID descptr);
STACK_PARAMS VOID  i386SetDescriptorAddress(PVOID descptr, LONG newaddr);
STACK_PARAMS VOID  i386SetDescriptorLimit(PVOID descptr, LONG lim);

STACK_PARAMS VOID i386AllowRing3IO(VOID);
STACK_PARAMS VOID i386MonitorRing3IO(VOID);

#endif /* I386_H */

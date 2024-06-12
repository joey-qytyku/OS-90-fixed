/*
  ����������������������������������������������������������������������������ͻ
  �                   Copyright (C) 2023-2024, Joey Qytyku                     �
  ����������������������������������������������������������������������������͹
  �  This file is part of OS/90 and is published under the GNU General Public  �
  �    License version 2. A copy of this license should be included with the   �
  �      source code and can be found at <https://www.gnu.org/licenses/>.      �
  ����������������������������������������������������������������������������ͼ
*/

#include <osk/sd/task.h>
#include <osk/sd/basicatomic.h>

VOID S_Terminate(PTASK pt)
{}

VOID S_ExecKernelThread(KTHREAD_PROC kp, PVOID pass_args)
{}

PTASK S_NewTask(VOID)
{}

VOID S_Yield(VOID)
{}

VOID S_Sched(PTASK pt)
{}

VOID S_Deactivate(PTASK pt)
{}

VOID S_SelfTerminate(VOID)
{}

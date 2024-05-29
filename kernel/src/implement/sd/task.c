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

HTASK TsT2Sd_CreateKernelThread(KTHREAD_PROC ktp)
{

}

//
// Completely expunges the task after calling the exit hook.
//
LONG TsT2Sd_TerminateTask(HTASK handle)
{
        PTASK pt = (PTASK)handle;

}

//
// Returns whatever is passed to it because handles are pointers.
// Always use this in driver code.
//
TASK *TsT2_TASKPtrFromHTASK(HTASK handle)
{
        return (TASK*)handle;
}

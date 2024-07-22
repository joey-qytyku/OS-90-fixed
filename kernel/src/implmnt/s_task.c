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

#include <OSK/SD/task.h>

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

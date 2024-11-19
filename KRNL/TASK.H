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


#ifndef TASK_H
#define TASK_H

typedef VOID (*KTHREAD_PROC)(PVOID);

typedef BOOL (*T0_TASK_PREHOOK)(STDREGS*);
typedef VOID (*T0_TASK_POSTHOOK)(STDREGS*);

// The task exit hook runs in T2, not T0.
typedef VOID (*T2_TASK_EXITHOOK)(STDREGS*);

typedef VOID (*T2_TASK_HND_EXCEPTION)(LONG, LONG);

typedef struct  Task_ {
	STDREGS regs;
	PVOID   _next;
	PVOID   _prev;
	SHORT   _time_slices;
	SHORT   _counter;
	LONG    flags;

	T0_TASK_PREHOOK pre;
	T0_TASK_POSTHOOK post;
	T2_TASK_EXITHOOK onexit;

	char name[8]; // Need this? I guess for debugging.
}TASK, *PTASK;

#define TFLAG_MASK 0b111

static inline PTASK GET_CURRENT_TASK(VOID)
{
	register LONG _ESP __asm__("esp");
	return (PTASK)(_ESP & (~4095));
}

#endif /* TASK_H */

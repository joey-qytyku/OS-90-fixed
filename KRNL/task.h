/////////////////////////////////////////////////////////////////////////////
//                     Copyright (C) 2022-2024, Joey Qytyku                //
//                                                                         //
// This file is part of OS/90.                                             //
//                                                                         //
// OS/90 is free software. You may distribute and/or modify it under       //
// the terms of the GNU General Public License as published by the         //
// Free Software Foundation, either version two of the license or a later  //
// version if you chose.                                                   //
//                                                                         //
// A copy of this license should be included with OS/90.                   //
// If not, it can be found at <https://www.gnu.org/licenses/>              //
/////////////////////////////////////////////////////////////////////////////

#ifndef TASK_H
#define TASK_H

typedef VOID (*KTHREAD_PROC)(PVOID);

typedef BOOL (*T0_TASK_PREHOOK)(PREGS);
typedef VOID (*T0_TASK_POSTHOOK)(PREGS);

// The task exit hook runs in T2, not T0.
typedef VOID (*T2_TASK_EXITHOOK)(PREGS);

typedef VOID (*T2_TASK_HND_EXCEPTION)(LONG, LONG);

#pragma pack(1)

typedef struct  Task_ {
	REGS    regs;
	PVOID   _next;
	PVOID   _prev;
	LONG    _switch_action;
	SHORT   _time_slices;
	SHORT   _counter;

	PVOID   vm;
	LONG    flags;

	T0_TASK_PREHOOK pre;
	T0_TASK_POSTHOOK post;
	T2_TASK_EXITHOOK onexit;

	char name[8]; // Need this? I guess for debugging.
}TASK;
#pragma pack()

#define TFLAG_MASK 0b111

static inline PTASK GET_CURRENT_TASK(VOID)
{
	register unsigned _ESP __asm__("esp");
	return (PTASK)(_ESP & (~4095));
}

VOID CreateTestTask(    TASK *	t,
			TASK *	next,
			TASK *	prev,
			VOID (*tp)(PVOID)
			);

VOID RunTaskTests(VOID);

#endif /* TASK_H */

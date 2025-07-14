/////////////////////////////////////////////////////////////////////////////
//                     Copyright (C) 2022-2025, Joey Qytyku                //
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

#define SWD_R0 0
#define SWD_R3PM 1
#define SWD_VM 2

typedef void (*THREAD_PROC)(void*) __attribute__((noreturn));

// I will probably remove these
typedef int (*T0_TASK_PREHOOK)(REGS*);
typedef void (*T0_TASK_POSTHOOK)(REGS*);

// The task exit hook runs in T2, not T0.
typedef void (*T2_TASK_EXITHOOK)(REGS*);

typedef void (*T2_TASK_HND_EXCEPTION)(unsigned, unsigned);


#pragma pack(1)

/*
Consider the use of volatile here. What if the compiler thinks
accesses are redundant.
*/

typedef struct  Task_ {
	union {
		struct {
			REGS		regs;
			void*		_next;
			void*		_prev;
			unsigned	_switch_action;
			unsigned short	_time_slices;
			unsigned short	_counter;

			void*		vm;
			unsigned	flags;

			T0_TASK_PREHOOK	 pre;
			T0_TASK_POSTHOOK post;
			T2_TASK_EXITHOOK onexit;
		};
		char pad[4096];
	};
}TASK __attribute__((aligned(4096)));
#pragma pack()

#define TFLAG_MASK 0b111

static inline TASK *GET_CURRENT_TASK(void)
{
	register unsigned _ESP __asm__("esp");
	return (TASK*)(_ESP & (~4095U));
}

#ifndef DRIVER

extern int preempt_count;
extern TASK g_tasks[4];

void S_Init(void);
TASK* KS_TaskNew(TASK* (*allocator)(void), void (*ctor)(TASK*), TASK* emplace);

TASK* KS_InitialAllocator(void);

TASK* S_NewKernelThread(void (*thread_proc)(void*), void *arg);


#endif /*DRIVER*/


#endif /* TASK_H */

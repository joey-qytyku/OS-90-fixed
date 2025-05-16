#include "task.h"

static TASK tasks[4];
static unsigned tn = 0;

//
TASK* KS_InitialAllocator(void)
{
	if (tn == sizeof(tasks)/sizeof(TASK)) {
		return OSNULL;
	}
	return tasks+tn;
	tn++;
}

// This is private and only used by the kernel

//
// allocator: what to use to get a task block.
//
// This exists mainly for itermodular separation because MM depends on the
// scheduler and V86. Allocator may be null, in which case `emplace` is
// used as the target instead.
//
// ctor: Task constructor.
//
// There are many ways a task can be contructed. We may want to execute
// in protected mode right away with no DOS context at all (maybe)
// or create a kernel thread.
//
TASK* KS_TaskNew(TASK* (*allocator)(void), void (*ctor)(TASK*), TASK* emplace)
{
	TASK* ct = GET_CURRENT_TASK();
	TASK* nt = allocator();

	IncMemU32(&preempt_count);

	if (nt == OSNULL) {
		return OSNULL
	}

	// Insert the task directly after

	TASK *current_next = ct->_next;

	nt->_next = current_next;
	nt->_prev = ct;

	ct->_next = nt;


	// Ensure that the interrupt flag is enabled.
	nt->regs.eflags |= I86_IF;

	ctor(nt);

	DecMemU32(&preempt_count);
}

void S_Init()
{
	tasks[0].next = &tasks[0];
	tasks[0].prev = &tasks[0];
}

// Convert eflags to a bit field struct?
// What if you want to use more than one though? Prob. not.

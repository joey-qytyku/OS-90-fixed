#include "task.h"

TASK g_tasks[4]__attribute__((aligned(4096)));

static unsigned tn = 0;

TASK* KS_InitialAllocator(void)
{
	if (tn == sizeof(g_tasks)/sizeof(TASK)) {
		return OSNULL;
	}
	return g_tasks+(tn++);
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
// Get rid of constructors and do it inline?
TASK* KS_TaskNew(TASK* (*allocator)(void), void (*ctor)(TASK*), TASK* emplace)
{
	TASK* ct = GET_CURRENT_TASK(); // This probably is incorrect on entry
	TASK* nt = allocator();

	IncMemU32(&preempt_count);

	if (nt == OSNULL) {
		return OSNULL;
	}

	// Insert the task directly after

	TASK *current_next = ct->_next;

	nt->_next = current_next; // Correct?
	nt->_prev = ct;

	ct->_next = nt;

	// Ensure that the interrupt flag is enabled.
	nt->regs.EFLAGS |= I86_IF;

	if (ctor != OSNULL) {
		ctor(nt);
	}

	DecMemU32(&preempt_count);

	return nt;
}

TASK* S_NewKernelThread(void (*thread_proc)(void*), void *arg)
{
	TASK* t = KS_TaskNew(KS_InitialAllocator, OSNULL, OSNULL);
	t->regs.EIP = (unsigned)thread_proc;
	t->regs.ESP = (UINT)memcpy((((void*)t) + 4096 - 4), &(UINT){(UINT)arg} ,4);
	t->_switch_action = SWD_R0;
}

void S_Init(void)
{
	g_tasks[0]._next = &g_tasks[0];
	g_tasks[0]._prev = &g_tasks[0];
}

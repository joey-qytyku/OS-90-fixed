#include "task.h"
#include "printf.h"

// Next and previous.

TASK t0;
TASK t1;
TASK t2;

VOID CreateTestTask
(
	PTASK CPTR_CDR t,
	PTASK CPTR_CDR next,
	PTASK CPTR_CDR  prev,
	VOID (*tp)(PVOID)
)
{
	t->regs.ESP = (LONG)t+4096;
	t->regs.EFLAGS = I86_IF;
	t->regs.EIP = (LONG)tp;
	t->_next = next;
	t->_prev = prev;
	t->_switch_action = 0;
}


VOID t1p(PVOID p)
{
	while (1)
		FuncPrintf(putE9, "Thread 1\n");
}

VOID t2p(PVOID p)
{
	while (1)
		FuncPrintf(putE9, "Thread 2\n");
}

VOID t3p(PVOID p)
{
	while (1)
		FuncPrintf(putE9, "Thread 3\n");
}

VOID RunTaskTests()
{
	CreateTestTask(&t0, &t1, &t2, t1p);
	CreateTestTask(&t1, &t2, &t0, t2p);
	CreateTestTask(&t2, &t0, &t1, t3p);
}

// VOID S_Yield()
// {

// }

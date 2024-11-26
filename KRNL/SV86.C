#include "SV86.H"
#include "PRINTF.H"

/*

Here is how it all works.

There is an array of handlers for each V86 INT vector. This is not necessary
but is faster.

There is a chain of hooks. They return-chain by calling the previous one
and either calling the previous one or returning to "eat" the request.
The return value of each handler can be 1 to indicate (HANDLED) or 0 to
indicate (REFLECT).

>>> THIS IS WRONG ACTUALLY

There is a stub handler that automatically routes the interrupt to real mode
in that special case.

*/

// Stub handler? Yes.
static HV86 v86_handlers[256];

static BOOL Stub(PREGS r)
{
	(VOID)r;
	return 1;
}

static inline VOID Pushw(PREGS r, SHORT v)
{
	r->SP -= 2;
	*(PSHORT)(r->SS * 16 + r->SP) = v;
}

static inline SHORT Popw(PREGS r)
{
	SHORT v = *(PSHORT)(r->SS*16 + r->SP);
	r->SP += 2;
	return v;
}

/*
The steps:

Call the handler chain. If it returns 1, reflection is needed.

To reflect:
- Set current vector to enter var to requested
- INT level counter set to 0

Loop:
- Push stuff into stack
- Enter V86
- If caught an INT
	- Level counter increment
	- Set vector to execute to the INT
	- Repeat loop
- If caught IRET
	- Decrement counter
	- If zero, we are done (clean the stack too)
	- If not zero, copy from stack to registers

*/

LONG V86xH(BYTE v, PREGS r)
{
	LONG int_caught;
	LONG rval;
	LONG level = 0;

	// If the vector has a handler, we do not have to do anything.
	// If the handler wants to call this function again that is fine
	// because of reentrancy.
	if (v86_handlers[v](r) == 0)
		return r->EAX & 0xFFFF;

	// Otherwise we will go straight into SV86.

	// DO NOT RUN THE THING TWICE
DoInt:

	Pushw(r, r->FLAGS);
	Pushw(r, r->CS);
	Pushw(r, r->IP);
	r->IP = IVT[v].ip;
	r->CS  = IVT[v].cs;

	// We are now in one level.

	level++;

ContInLevel:
	int_caught = EnterV86(&r);
	FuncPrintf(putchar, "\tCaught: %x\n", int_caught);


	// Is the INT caught an INT x call and not an IRET?
	// If so, repeat the loop and perform emulation
	// This is also given that there was no handler capable of the request.
	if (int_caught < 256 && v86_handlers[v](r) == 1)
		goto DoInt;

	// Otherwise, we caught an IRET or just doing the obligatory
	// stack emulation since a captured INT already did "return."

	// Caught an IRET in this case
	level--;
	if (level == 0)
		return r->EAX & 0xFFFF;

	// Simulate IRET on the stack
	r->IP           = Popw(r);
	r->CS           = Popw(r);
	r->FLAGS        = Popw(r);

	// Continue without doing the whole
	// Interrupt frame generation because there is
	// no interrupt.
	goto ContInLevel;
}

VOID InitV86(VOID)
{
	for (int i = 0; i < 256; i++)
		v86_handlers[i] = Stub;
}

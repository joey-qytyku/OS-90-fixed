#include "SV86.H"

/*

Here is how it all works.

There is an array of handlers for each V86 INT vector. This is not necessary
but is faster.

There is a chain of hooks. They return-chain by calling the previous one
and either calling the previous one or returning to "eat" the request.
The return value of each handler can be 1 to indicate (HANDLED) or 0 to
indicate (REFLECT).

There is a stub handler that automatically routes the interrupt to real mode
in that special case.

*/

// Stub handler? Yes.
static HV86 v86_handlers[256];

static BOOL Stub(PSTDREGS r)
{
	(VOID)r;
	return 1;
}


LONG INTxH(BYTE v, PSTDREGS r)
{
	if (v86_handlers[v](r) == 1) {
		r->CS  = IVT[v].cs;
		r->EIP = IVT[v].ip;
		// Give it a stack too?
		LONG int_got = V86xH(r);

		if (int_got != 0xFFFFFFFFu) {
			return INTxH(int_got, r);
		}
	}
	return (LONG)r->AX;
}

VOID InitV86(VOID)
{
	for (int i = 0; i < 256; i++)
		v86_handlers[i] = Stub;
}

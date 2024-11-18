#include "M_CORE.H"
#include "SV86.H"

// First few entries correspond with the output of M_GetInfo.
struct mmctrl {
	LONG    emf;
	LONG    emp;

	LONG    dbf;
	LONG    dbp;

	LONG    vsf;
	LONG    vsp;

	LONG    cm;
};

static struct mmctrl mm;

VOID M_Init()
{}

PVOID M_XMAlloc(PVOID   force_addr,
		LONG    commit,
		LONG    uncommit,
		LONG    bits)
{

}


LONG M_GetInfo(BYTE class)
{
}

PVOID M_CMAlloc(LONG bytes)
{
	// Disable preemption here
}

VOID M_CMFree();



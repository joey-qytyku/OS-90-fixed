/*******************************************************************************
		      Copyright (C) 2022-2024, Joey Qytyku

  This file is part of OS/90.

  OS/90 is free software. You may distribute and/or modify it under
  the terms of the GNU General Public License as published by the
  Free Software Foundation, either version two of the license or a later
  version if you choose.

  A copy of this license should be included with OS/90.
  If not, it can be found at <https://www.gnu.org/licenses/>
*******************************************************************************
This file implements the DPMI call interface.

*******************************************************************************/

#define SET_CARRY(regs) {regs->EFLAGS |= 1;}
#define CLR_CARRY(regs) {regs->EFLAGS & (~1)}
#define TOFUNC(enum) enum##_impl

typedef VOID (FUNC_GROUP)(PEMU_CONTEXT, PTASK, PSTDREGS);

enum {
	DPMI_LDT        = 0x00,
	DPMI_DOSMEM     = 0x01,
	DPMI_INT        = 0x02,
	DPMI_XLAT       = 0x03,
	DPMI_VER        = 0x04,
	DPMI_XMEM       = 0x05,
	DPMI_PAGE       = 0x06,
	DPMI_DEMAND     = 0x07,
	DPMI_PHYSMAP    = 0x08,
	DPMI_IFCTX      = 0x09,
	DPMI_VENDOR     = 0x0A,
	DPMI_DEBUG      = 0x0B,
	DPMI_TSR32      = 0x0C,
	DPMI_SHMEM      = 0x0D,
	DPMI_FPUEMU     = 0x0E,
	DPMI_NUM_GROUPS
};

static VOID TOFUNC(DPMI_LDT)(PEMU_CONTEXT e, PTASK t, PSTDREGS, r)
{

}

static VOID TOFUNC(DPMI_DOSMEM)(PEMU_CONTEXT e, PTASK t, PSTDREGS, r)
{
}

// AH=05
static VOID TOFUNC(DPMI_XMEM)(PEMU_CONTEXT e, PTASK t, PSTDREGS, r)
{
}

static FUNC_GROUP func_groups[DPMI_NUM_GROUPS] = {
	[DPMI_LDT]      = TOFUNC(DPMI_LDT),
	[DPMI_DOSMEM]   = TOFUNC(DPMI_DOSMEM),
	[DPMI_INT]      = TOFUNC(DPMI_INT),
	[DPMI_XLAT]     = TOFUNC(DPMI_XLAT),
	[DPMI_VER]      = TOFUNC(DPMI_VER),
	[DPMI_XMEM]     = TOFUNC(DPMI_XMEM),
	[DPMI_PAGE]     = TOFUNC(DPMI_PAGE),
	[DPMI_DEMAND]   = TOFUNC(DPMI_DEMAND),
	[DPMI_PHYSMAP]  = TOFUNC(DPMI_PHYSMAP),
	[DPMI_IFCTX]    = TOFUNC(DPMI_IFCTX),
	[DPMI_VENDOR]   = TOFUNC(DPMI_VENDOR),
	[DPMI_DEBUG]    = TOFUNC(DPMI_DEBUG),
	[DPMI_TSR32]    = TOFUNC(DPMI_TSR32),
	[DPMI_SHMEM]    = TOFUNC(DPMI_SHMEM),
	[DPMI_FPUEMU]   = TOFUNC(DPMI_FPUEMU)
}

static PVOID Handle21h(PSTDREGS r)
{
	switch (r->AH)
	{
		case :
	}
}

VOID D_Init()
{
	HookINTxH(0x21, &int21h_prevhook, Handle21h);
}

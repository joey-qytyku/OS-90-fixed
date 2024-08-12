#include <KRNL/DESC.H>

enum {
	GDT_NULL,
	GDT_CODE,
	GDT_DATA,
	GDT_LDT,
	GDT_TSS,
	GDT_ENTRIES
};

// NOTE: 40H must be reserved for Windows
// to be used for BDA.

static SEGMENT_DESCRIPTOR gdt[GDT_ENTRIES];

static SEGMENT_DESCRIPTOR ldt[128];

// I only need to support what DPMI requires.
// That means focus more on the LDT.
// Feel free to define the GDT right here.

// VOID SetDescBase()
// {}

// VOID SetDescSizePg()

// VOID SetDescSizeBytes(PSEGMENT_DESCRIPTOR sd, LONG bytes)
// {}

// VOID SetDescAccessRights(BYTE ring, BYTE present)

// VOID CreateRzSegment(BYTE type)

VOID L_SetSegmentBase(LONG base)
{}

VOID L_SetLdtSegBase(SHORT selector, LONG base);

VOID L_LdtAlloc()
{}

VOID L_InitKernelSegments()
{
}

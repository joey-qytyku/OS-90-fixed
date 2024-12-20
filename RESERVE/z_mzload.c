#include "m_core.h"

typedef struct {
	SHORT   e_magic;
	SHORT   e_cblp;
	SHORT   e_cp;
	SHORT   e_crlc;
	SHORT   e_cparhdr;
	SHORT   e_minalloc;
	SHORT   e_maxalloc;
	SHORT   e_ss;
	SHORT   e_sp;
	SHORT   e_csum;
	SHORT   e_ip;
	SHORT   e_cs;
	SHORT   e_lfarlc;
	SHORT   e_ovno;
	SHORT   e_res[4];
	SHORT   e_oemid;
	SHORT   e_oeminfo;
	SHORT   e_res2[10];
	LONG    e_lfanew;
}MZ_HEADER, *PMZ_HEADER;

static MZLoad(char *full_path)
{
	// Open the file

	// Get the size of the file by seeking to the end

	// Seek to start

	// Allocate TMP header (32 bytes)
	// Allocate the PSP, 256 bytes

	// Read the header into the allocated buffer

	// Get another buffer for the full relocation table
	// - TMP reloc  e_crlc * 4

	// Copy the relocation table to memory.

	// Allocate the program data:
	// - (e_cp-1) * 512 + e_cblp + e_minalloc

	// Copy the program data into memory from the file

}

LONG PrepareDosLoad(PREGS ctx)
{
}

#include <OSK/exec.h>
#include <OSK/mm.h>


// Will do backward copy to page align the code
// Actually, it is a smarter idea to copy the header first and to a separate
// buffer, as it is not that large. This way, the relocations and symbols
// can be chopped out easily.
// It can also be placed above everything else to be easily discarded.

// Also, I should support a pagable code segment.

STAT LoadExec(PEXEC_LOADER el, PBYTE path, LONG page_flags)
{
	LONG fh = 0;

	if ( (fh = el->fopen(path)) == 0) {
		printf("Error opening executable at path %s\n", path);
		return OS_ERR;
	}

	// Get size of file to allocate buffer
	LONG size = el->fseek(fh, EXEC_SEEK_END, 0) + 1;

	// Allocate pages
	LONG  ch        = M_Alloc(size, 0, page_flags);
	PVOID buff      = M_ReserveMapping(size);
	STAT  mapped    = M_Map(buff, ch);

	if (ch == 0 || buff == NULL || mapped == 1) {
		printf("Error allocating memory for executable %s\n", path);
		return OS_ERR;
	}

	// Seek back to zero
	el->fseek(fh, EXEC_SEEK_SET, 0);

	// Read MZ header first
	el->fread(fh, buff, sizeof(MZ_HEADER));

	// Is it valid?
	if (WORD_PTR(buff,0) != 0x5A4D) {
		printf("Invalid MZ magic number in %s\n", path);
		return OS_ERR;
	}

	// Seek to very end of MZ stub
	el->fseek(fh, ((PMZ_HEADER)buff)->e_cblp + ((PMZ_HEADER)buff)->e_cp * 512);

	// File pointer here is at the EXEC_HEADER. Read to end of buffer
	// so that it can be freed easily when no longer needed.
	el->fread(
		fh,
		buff + (size - sizeof(EXEC_HEADER) - 1),
		sizeof(EXEC_HEADER)
	);

	// Read code/data, symbols, etc. into beginning of buffer
	// File pointer is NOT actually there though.
	// It must be aligned at a 512-byte boundary
	{
		LONG align_pos = el->fseek(EXEC_SEEK_CUR, 0);
		align_pos = (align_pos + 511) & (~511);
		el->fseek(EXEC_SEEK_SET, align_pos);
	}

	// Now we can read the whole file MINUS both headers
	// Partial read will be done and that is fine.
	el->fread(fh, buff, 0xFFFFFFFF);

	// Layout is like this:
	// - Code/data section
	// - Rela
	// - Rel
	// - SRT
	// - Strtab
	// - Import table


}

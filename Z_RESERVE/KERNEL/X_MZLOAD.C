#include "X_DEFS.H"

// Uses the same executable loader struct as EF90 but no
// PTRTAB. Path must be full including drive letter.
STAT X_MZLoad(
	PEXEC_LOADER    ldr,
	PCSTR           path
)
{
	MZ_HEADER hdr;
	LONG handle;
	LONG status = 0;

	ldr->fopen(path);
	ldr->fread(handle, &hdr, sizeof hdr);

	if (hdr->e_magic != 'MZ') {
		status = 1;
		goto cleanup;
	}
	// Use ASM and variable conventions?


	cleanup:
	return status;
}

#include <coff.h>
#include "KRNL/type.h"

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_STATIC_SECTS 16

typedef struct {
	FILHDR		hdr;
	AOUTHDR		hdr2;
	SCNHDR*		sections_array;
	RELOC**		sections_relocs;
	RELOC*		section_relocs_default_tab[MAX_STATIC_SECTS];
}COFF_REPR;

// Incorrect pointer
COFF_REPR *COFF_New(int fd)
{
	COFF_REPR *C = (COFF_REPR*)malloc(sizeof(COFF_REPR));

	if (C == NULL) {
		return NULL;
	}

	int bytes_read = read(fd, &C->hdr, FILHSZ);

	if (bytes_read != FILHSZ || C->hdr.f_magic != I386MAGIC) {
		free(C);
		return NULL;
	}

	// Read optional header. Comes directly after.

	bytes_read = read(fd, &C->hdr2, C->hdr.f_opthdr);
	if (bytes_read != AOUTSZ)
		goto free_C;

	// We will not represent all sections in the file. //
	// We are already there right now.
	// Allocate enough section structures with malloc
	C->sections_array = malloc(SCNHSZ * C->hdr.f_nsyms);

	// There will always be at least _DATA and _TEXT.
	// Zero sections is impossible.
	if (C->sections_array == NULL)
		goto free_C;

	// Prepare the section relocation array pointer table.
	if (C->hdr.f_nscns > MAX_STATIC_SECTS) {
		C->sections_relocs = malloc(sizeof(RELOC*)*C->hdr.f_nscns);
	}
	else {
		C->sections_relocs = &C->section_relocs_default_tab;
	}


	for (DWORD i = 0; i < C->hdr.f_nscns; i++) {
		bytes_read = read(fd, C->sections_array+i, SCNHSZ);

		if (bytes_read != SCNHSZ) {
			goto free_C_Sects;
		}

		C->sections_relocs[i] = malloc(C->sections_array[i].s_nreloc*RELSZ);

		long save = lseek(fd, 0, SEEK_CUR);
		lseek(fd, C->sections_array[i].s_relptr, SEEK_SET);

		read(fd, C->sections_relocs[i], RELSZ * C->sections_array[i].s_nreloc);
	}

	/***********************************************************************
	 * The sections are represented in memory and all relocations are also
	 * loaded in memory successfully. The section data is NOT loaded.
	***********************************************************************/

	return C;

	free_C:
	free(C);
	return NULL;

	free_C_Sects:
	free(C->sections_array);
	free(C);
	return NULL;
}

/*******************************************************************************
 * Reads section data, loads it into memory, and applies all relocations.
*******************************************************************************/
int COFF_LoadDynSectToAddr(unsigned index)
{
}

int main(int argc, char **argv)
{
	if (argv[1] == NULL) {
		return EXIT_FAILURE;
	}
	int fd = open(argv[1], O_RDONLY);

	if (fd == -1) {
		return EXIT_FAILURE;
	}
}


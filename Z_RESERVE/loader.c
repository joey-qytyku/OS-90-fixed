#include <coff.h>
#include "KRNL/type.h"

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#define MAX_STATIC_SECTS 16

#define coffdeb(x, ...) printf(x, __VA_OPT__(,) __VA_ARGS__)

// NEVER STATICALLY ALLOCATE.
typedef struct {
	FILHDR		hdr;
	AOUTHDR		hdr2;
	SCNHDR*		sections_array;
	SCNHDR		sections_default_array[MAX_STATIC_SECTS];
	RELOC**		sections_relocs;
	RELOC*		section_relocs_default_tab[MAX_STATIC_SECTS];
	char*		strtab;
	SYMENT*		symtab;
	unsigned	strsize;
}COFF_REPR;

// Can I avoid this difficulty of lifetimes by preprocessing the executable
// somehow? Maybe use one giant loader function.

//
// Completely safe function to free all memory used by a loader instance.
// Can be called at any time with no errors. Checks before freeing.
//
// After running, the COFF_REPR object is invalid permanently.
//
static void FreeAll(COFF_REPR *C)
{
	if (	C->sections_array != NULL
		&& C->sections_array != C->sections_default_array
	) {
		free(C->sections_array);
	}

	// sections_relocs points to an array of pointers.
	// This array must be deallocated along with each
	// buffer pointed.

	for (unsigned i = 0; i < C->hdr.f_nscns; i++) {
		if (C->sections_relocs[i] != NULL) {
			free(C->sections_relocs[i]);
			C->sections_relocs[i] = NULL;
		}
	}

	if (	(
			(SCNHDR*__restrict)C->sections_relocs
			!=
			(SCNHDR*__restrict) C->sections_default_array
		)
		&& C->sections_array != NULL
	) {
		// In this case, the list of reloc array pointers
		// was allocated separately, so free it.
		// We already checked to make sure it is in fact not NULL
		// because the init process may not actually reach allocating
		// the section relocs at all.
		free(C->sections_array);
		C->sections_array = NULL;
	}

	// Delete the rest of the structure.
	free(C);
}

// If the pointer is NULL, failed and all memory is deallocated.

COFF_REPR *COFF_New(int fd)
{
	COFF_REPR *C = (COFF_REPR*)malloc(sizeof(COFF_REPR));

	// Init pointers to NULL for safety!!!

	lseek(fd, 0, SEEK_SET);

	if (C == NULL) {
		return NULL;
	}

	int bytes_read = read(fd, &C->hdr, FILHSZ);

	if (bytes_read != FILHSZ || C->hdr.f_magic != I386MAGIC) {
		free(C);
		return NULL;
	}

	// Read optional header. Comes directly after.
	// It can be of the types: AOUTHDR for an executable or GNU_AOUT.
	//
	// AOUTHDR is 28 bytes and GNU_AOUT is 32. GNU is undefined and just
	// has to be skipped over.
	//
	// f_opthdr gives the size and the type.
	// It may be zero too for object files.
	//

	printf("f_opthdr:%u\n", C->hdr.f_opthdr);

	if (C->hdr.f_opthdr == 28) {
		bytes_read = read(fd, &C->hdr2, 28);

		if (bytes_read != AOUTSZ || C->hdr2.magic != ZMAGIC) {
			puts("Failed to read optional header.");
			goto Fail;
		}
	}
	else if (C->hdr.f_opthdr == 32) {
		// Skip it because the GNU_AOUT header defines nothing
		// important.
		lseek(fd, 32, SEEK_CUR);
	}
	else {
		printf("No optional header\n");
		// Otherwise it has no optional header. Indicate that by setting
		// the magic to 0 which is always invalid.
		C->hdr2.magic = 0;
	}

	printf("Read optional header\n");

	// We will now represent all sections in the file.
	// We are already there right now.
	// Allocate enough section structures with malloc
	C->sections_array = malloc(SCNHSZ * C->hdr.f_nsyms);

	printf("Number of sections: %u\n", C->hdr.f_nscns);

	// There will always be at least _DATA and _TEXT.
	// Zero sections is impossible.

	if (C->sections_array == NULL) {
		goto Fail;
	}

	// Prepare the section relocation array pointer table.
	if (C->hdr.f_nscns > MAX_STATIC_SECTS) {
		C->sections_relocs = malloc(sizeof(RELOC*)*C->hdr.f_nscns);
	}
	else {
		C->sections_relocs = C->section_relocs_default_tab;
	}

	// The section table is contiguous and directly after the headers.
	// Read the whole thing into the memory.
	read(fd, C->sections_array, SCNHSZ * C->hdr.f_nscns);

	// We do not need to seek back after reading.

	// Allocate space for the relocation tables and read them.

	for (unsigned i = 0; i < C->hdr.f_nscns; i++) {
		C->sections_relocs[i] = malloc(C->sections_array[i].s_nreloc*RELSZ);

		// Move to the relocations of this section
		lseek(fd, C->sections_array[i].s_relptr, SEEK_SET);

		// Read relocations into the buffer
		read(fd, C->sections_relocs[i], RELSZ * C->sections_array[i].s_nreloc);
	}

	/***********************************************************************
	 * The sections are represented in memory and all relocations are also
	 * loaded in memory successfully. The section data is NOT loaded.
	***********************************************************************/

	return C;

	Fail:
	FreeAll(C);
	return NULL;
}
/*******************************************************************************
 * Symbols are not loaded automatically. This will buffer all symbols and the
 * string table.
*******************************************************************************/
void COFF_LoadSyms(COFF_REPR *C, int fd)
{
	// Allocate space for the symbol entries.
	C->symtab = malloc(C->hdr.f_nsyms * SYMESZ);

	// Read all of them
	lseek(fd, C->hdr.f_symptr, SEEK_SET);
	read(fd, C->symtab, C->hdr.f_nsyms * SYMESZ);

	getchar();

	lseek(fd, C->hdr.f_symptr + C->hdr.f_nsyms * SYMESZ, SEEK_SET);

	unsigned i = 0;
	read(fd, &i, 4);

	printf("SIZE: %u\n", i);

	C->strtab = (char*)malloc(i);

	memset(C->strtab, 0, 4);
	read(fd, C->strtab+4, i-4);

	C->strsize = i;
}

void COFF_Del(COFF_REPR *C, int fd)
{
	FreeAll(C);
	close(fd);
}

/*******************************************************************************
 * Reads section data, loads it into memory, and applies all relocations.
 *
*******************************************************************************/
int COFF_LoadDynSectToAddr(unsigned index)
{
}

static void Tests(COFF_REPR *C, int fd)
{
	if (C == NULL) {
		puts("Failed to represent object");
		return;
	}

	// If the section name is too long, a / is inserted
	for (unsigned i = 0; i < C->hdr.f_nscns; i++) {
		printf("%-40.8s%40lu",
			C->sections_array[i].s_name,
			C->sections_array[i].s_size);
	}
	getchar();

	// Slash naming is ONLY used for sections, not symbols.

	for (unsigned int i = 0; i < C->hdr.f_nsyms-1; i++) {
		if (i % 25 == 0) {
			getchar();
		}
		if (C->symtab[i].e.e.e_zeroes == 0 && C->symtab[i].e.e.e_offset < C->strsize) {
			// getchar();
			// fprintf(stderr, "%u:", C->hdr.f_symptr+C->symtab[i].e.e.e_offset);
			fprintf(stderr, "%s\n", C->strtab + C->symtab[i].e.e.e_offset);
		}
		else if (C->symtab[i].e.e.e_zeroes != 0) {
			fprintf(stderr, "%.8s\n", C->symtab[i].e.e_name);
		}
	}
}

void COFF_SymNameCmp(unsigned index, const char *)
{}
//
// The callbacks are invoked for each symbol that is imported or exported.
// ri will request the symbol address from the same.
// rx will request that the symbol is exported.
//
// OS/90 drivers
//
void COFF_ResolveRelocsAndImpsExps
(
	COFF_REPR *C,
	int (*ri)(),
	int (*rx)()
)
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

	COFF_REPR *C = COFF_New(fd);
	COFF_LoadSyms(C, fd);
	// Null string problem?
	// printf("%s", C->symtab+4);
	Tests(C, fd);

	// COFF_Del(C, fd);
}


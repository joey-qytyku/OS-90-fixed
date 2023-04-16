#include "exetool.h"

int init_exetool(exe_util *ex, const char *infile_name)
{
    struct stat st;

    ex->infile_fd = open(infile_name, O_RDONLY);

    // We should not stat if file does not exist
    if (ex->infile_fd == -1)
    {
        perror("exetool");
        return 1;
    }

    if (stat(infile_name, &st) == -1)
    {
        perror("exetool");
    }

    ex->size = st.st_size; //?

    ex->mapped_infile = mmap(
        NULL,
        st.st_size,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE,
        ex->infile_fd,
        0
    );

    if (ex->mapped_infile == NULL)
    {
        perror("exetool");
        return 1;
    }

    ex->mz_hdr      = ex->mapped_infile;
    ex->pe_main_hdr = ex->mapped_infile + ex->mz_hdr->e_lfanew;
    ex->pe_opt_hdr  =
        ex->mapped_infile +
        ex->mz_hdr->e_lfanew +
        sizeof(pe_header);

    // Data directories are part of the optional header
    // they are a list of 16 structures
    // Directly after is the section table

    ex->data_dirs = ex->pe_opt_hdr + sizeof(pe_optional_header);
    ex->sections  = ex->pe_opt_hdr + ex->pe_main_hdr->mSizeOfOptionalHeader;

    printf(
        "[PE Main Header Information Dump]\n"
        "Machine               | 0x%x\n"
        "Size of opt hdr       | %i\n"
        "Number of sections    | %i\n\n",
        ex->pe_main_hdr->mMachine,
        ex->pe_main_hdr->mSizeOfOptionalHeader,
        ex->pe_main_hdr->mNumberOfSections // ???
    );

    printf(
        "[PE Optional Header Information Dump]\n"
        "RVA of  .text   | 0x%x\n"
        "Size of .text   | %i\n"
        "RVA of  .data   | 0x%x\n"
        "Size of .data   | %i\n"
        "Size of .bss    | %i\n",

        ex->pe_opt_hdr->mBaseOfCode,
        ex->pe_opt_hdr->mSizeOfCode,

        ex->pe_opt_hdr->mBaseOfData,
        ex->pe_opt_hdr->mSizeOfCode,
        ex->pe_opt_hdr->mSizeOfCode,
        ex->pe_opt_hdr->mSizeOfUninitializedData
    );
    return 0;
}

int close_exetool(exe_util *ex)
{
    munmap(ex->mapped_infile, ex->size);
    close(ex->infile_fd);
}

void test(exe_util *ex)
{
    printf("Number of sections: %i\n", ex->pe_main_hdr->mNumberOfSections);

    uint16_t num_sects = ex->pe_main_hdr->mNumberOfSections;

    for (int i = 0; i < num_sects; i++)
    {
        write(STDOUT_FILENO, ex->sections[i].mName, 8);
        putchar('\n');
        printf("Virtual address | 0x%x\n",ex->sections[i].mVirtualAddress);
        printf("Size of section | %i\n",  ex->sections[i].mVirtualSize);
        printf("Relocations     | %x\n",  ex->sections[i].mNumberOfRelocations);
        printf("Characteristics | 0x%x",  ex->sections[i].mCharacteristics);
    }
}

int main(int argc, const char** argv)
{
    if (argc != 8)
    {
        fprintf(stderr, "Invalid number of arguments (%i)\n",argc);
        return EXIT_FAILURE;
    }

    const char *stub_name = argv[1];
    const char *entry_sym = argv[2];
    const char *load_addr = argv[3];
    const char *stack_sym = argv[4];
    const char *out_name  = argv[5];
    const char *in_name   = argv[6];
    bool remove_fixups    = argv[7][0];

    exe_util ex;

    if (init_exetool(&ex, in_name))
    {
        fprintf(stderr, "Failed to initialize exetool\n");
        return EXIT_FAILURE;
    }

    test(&ex);
    close_exetool(&ex);

    return EXIT_SUCCESS;
}

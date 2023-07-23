#include <stdint.h>
#include <stdio.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <alloca.h>
#include <string.h>

typedef struct {
    uint32_t   magic;

    struct {
    uint32_t   text;
    uint32_t   data;
    uint32_t   bss;
    uint32_t   rela_text;
    uint32_t   rela_data;
    uint32_t   symtab;
    uint32_t   strtab;
    }rva;

    struct {
    uint32_t   text;
    uint32_t   data;
    uint32_t   bss;
    uint32_t   rela_text;
    uint32_t   rela_data;
    uint32_t   symtab;
    uint32_t   strtab;
    }bytes;

    uint32_t   entry;
}ESS_HEADER;

static off_t find_file_size(int fd)
{
    off_t val = lseek(fd, 0, SEEK_END);
    return val;
}

// One argument:
//    the path to the file to prepare. The file extension should be .ess.
//    output is DRV.
//
int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Invalid arguments\n");
        return 1;
    }

    int fd;
    off_t fsize;
    void *fpointer;

    fd = open(argv[1], O_RDWR);
    fsize = find_file_size(fd);

    if (fd == -1)
    {
        fprintf(stderr, "Invalid arguments\n");
        return 1;
    }

    fpointer = mmap(NULL, fsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (fpointer == MAP_FAILED)
    {
        fprintf(stderr, "Failed to map file\n");
        return 1;
    }

    ESS_HEADER *ehdr = fpointer + (fsize - sizeof(ESS_HEADER));

    if (ehdr->magic != 0xC3C3C3C3)
    {
        fprintf(stderr, "Could not find magic number at and of file\n");
        goto End;
    }
    printf("Found magic number.\n");

    // The procedure is to make space and save the header, copy the data
    // forward, and write the header to the begining.

    ESS_HEADER save = *ehdr;

    memmove(fpointer + sizeof(ESS_HEADER), fpointer, fsize - sizeof(ESS_HEADER));
    memcpy(fpointer, &save, sizeof(ESS_HEADER));

End:
    munmap(fpointer, fsize);
    close(fd);
}

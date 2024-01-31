#define MAGIC 0x5A4D

struct EXE {
    U16 signature; /* == 0x5a4D */
    U16 bytes_in_last_block;
    U16 blocks_in_file;
    U16 num_relocs;
    U16 header_paragraphs;
    U16 min_extra_paragraphs;
    U16 max_extra_paragraphs;
    U16 ss;
    U16 sp;
    U16 checksum;
    U16 ip;
    U16 cs;
    U16 reloc_table_offset;
    U16 overlay_number;
};

// The MZ executable will load everything in one contiguous block in memory.
// It does not have to be directly after the PSP and may in fact be anywhere
// since the PSP is passed by register upon entry. This requires the loader
// to set the state of the registers.

// Loader returns base segment of allocation, which goes in PSP
U16 Load_EXE(const char *file)
{}

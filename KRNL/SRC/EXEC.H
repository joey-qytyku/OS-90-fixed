#ifndef EXEC_H
#define EXEC_H

#define EXEC_FLAGS_LIB 1
#define EXEC_DUPABLE_DSEG 2

#define FPAGE_SIZE 512

typedef struct {
	WORD    e_magic;
	WORD    e_cblp;
	WORD    e_cp;
	WORD    e_crlc;
	WORD    e_cparhdr;
	WORD    e_minalloc;
	WORD    e_maxalloc;
	WORD    e_ss;
	WORD    e_sp;
	WORD    e_csum;
	WORD    e_ip;
	WORD    e_cs;
	WORD    e_lfarlc;
	WORD    e_ovno;
	WORD    e_res[4];
	WORD    e_oemid;
	WORD    e_oeminfo;
	WORD    e_res2[10];
	LONG    e_lfanew;
}MZ_HEADER, *PMZ_HEADER;

typedef struct {
	SHORT   magic;
	SHORT   flags;
	LONG    code_bytes;
	LONG    bss_bytes;
	LONG    srt_sdp_bytes;
	LONG    rel_num;
	LONG    rela_num;
}EXEC_HEADER,*PEXEC_HEADER;

typedef struct {
	SHORT   flags;
	SHORT   symbol;
	LONG    offset
	LONG    addend;
}EXEC_RELA,*PEXEC_RELA;

typedef struct {
	SHORT   flags;
	SHORT   symbol;
	LONG    offset;
}EXEC_REL,*PEXEC_REL;

typedef struct {
        SHORT longs;
        SHORT srt_index;
}EXEC_SRT_ENTRY;

// Points to all structures created when in the load phase.
// Used internally by loader. There is no need to check the executable
// header because all of the program features are

// Must be no larger than 32 bytes, or 8 longs.
typedef struct {
	LONG                    srt_num;
	PEXEC_STR_ENTRY         srt;
	SHORT                   rel_num;
	SHORT                   rela_num;
	PEXEC_REL               rel;
	PEXEC_RELA              rela;
}EXEC_LDR_FILE_PTRTAB; // Do not make ptr typedef

//// Finalize load commands?
#define EXEC_OPEN_RING3PAGES 1
#define EXEC_OPEN_UNLOCKED_DSEG 2

#define EXEC_SEEK_SET 0
#define EXEC_SEEK_CUR 1
#define EXEC_SEEK_END 2

FNPTR(LONG, EXEC_FILE_OPENER,   (PCSTR path) );
FNPTR(LONG, EXEC_FILE_CLOSER,   (LONG handle) );
FNPTR(LONG, EXEC_FILE_SEEK,     (LONG handle, LONG seek) );
FNPTR(LONG, EXEC_FILE_READ,     (LONG handle, PVOID buff, LONG size) );
FNPTR(SHORT,EXEC_FILE_LOWALLOC, (SHORT pgc));

typedef struct {
	EXEC_FILE_OPENER        fopen;
	EXEC_FILE_CLOSER        fclose;
	EXEC_FILE_SEEK          fseek;
	EXEC_FILE_READ          fread;
	EXEC_FILE_PTRTAB        ptrtab;
	EXEC_FILE_LOWALLOC      lowalloc;
}EXEC_LOADER, *PEXEC_LOADER;

#endif /* EXEC_H */

#pragma pack(1)

typedef struct MBR {
	unsigned char _boot_code[446];
	struct {
		#define MBR_ATTR_BOOT_DISK 0x80
		unsigned char attribute;

	} partition[];
};

struct f1216 {
	// Number to use for INT 13H. This is useless because the boot sector
	// starts with CL=boot drive. Maybe useful for detecting if the disk is
	// in the wrong drive.
	unsigned char	BS_DrvNum;
	unsigned char	BS_Reserved1; // Set to zero

	// 0x29 if the next 3 fields are non-zero and valid.
	unsigned char	BS_BootSig;
	unsigned int	BS_VolID; // 32-bit value of current time of formatting

	// "volume label" is just a name for the user. It actually may
	// be checked by some AV software for modification.
	char		BS_VolLab[11];

	// "FATxx   ". Not to be relied on and does not actually define the
	// FAT type.
	char		BS_FilSysType[8];
};

struct f32 {
	//
	//
	//
	unsigned	BPB_FATSz32;
	unsigned short	BPB_ExtFlags;
	unsigned short	BPB_FSVer; // BCD, usually set to zero anyway.
	unsigned	BPB_RootClus;
	unsigned short	BPB_FSInfo;
	unsigned short	BPB_BkBootSec;
	unsigned char	BPB_Reserved[12];
	unsigned char	BS_DrvNum;
	unsigned char	BS_Reserved1;
	unsigned char	BS_BootSig;
	unsigned int	BS_VolID;
	char		BS_VolLab[11];
	char		BS_FilSysType[8];
};

typedef struct
{
	char		BS_jmpBoot[3];
	char		BS_OEMName[8];
	unsigned short	BPB_BytsPerSec; // Can be 512, 1024, 2048, or 4096
	char		BPB_SecPerClus;
	char		BPB_RsvdSecCnt[2];
	unsigned short	BPB_NumFATs;
	unsigned short	BPB_RootEntCnt;
	unsigned short	BPB_TotSec16;
	char		BPB_Media;

	// Zero on FAT32 and nonzero on FAT12 and FAT16
	unsigned short	BPB_FATSz16;

	// The following are not to be relied on because the BIOS gives this
	// information. Sometimes the formatting tool does not even know the
	// correct parameters. The FAT FS is basically LBA-based and has no
	// other dependency on CHS.
	unsigned short	BPB_SecPerTrk;
	unsigned short	BPB_NumHeads;
	unsigned int	BPB_HiddSec;
	unsigned int	BPB_TotSec32;

	// Past this point is the extended BPB
	// Different structure between F32 and F16/F12

	union {
		struct f32 f32;
		struct f1216 f1216;
	};
}BPB;

// Determining the FAT type is done by counting the number of clusters in the
// FAT. This is the only legal way to do it.
//
// FAT12 may not have more than 4084 sectors and FAT16 must have more.
// FAT32 is simpler and requires only checking if the root dir entries are
// zero, which they must be because it has a resizable root dir.
//

typedef struct {
	#define ATTR_READ_ONLY	0x01
	#define ATTR_HIDDEN	0x02
	#define ATTR_SYSTEM	0x04
	#define ATTR_VOLUME_ID	0x08
	#define ATTR_DIRECTORY	0x10
	#define ATTR_ARCHIVE	0x20
	#define ATTR_LONG_NAME \
	(ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID)

	char		DIR_Name[11];
	unsigned char	DIR_Attr;
	char		DIR_NTRes;
	unsigned char	DIR_CrtTimeTenth;
	unsigned short	DIR_CrtTime;
	unsigned short	DIR_CrtDate;
	unsigned short	DIR_LstAccDate;

	unsigned short	DIR_FstClusHI;
	unsigned short	DIR_WrtTime;
	unsigned short	DIR_WrtDate;
	unsigned short	DIR_FstClusLO;
	unsigned int	DIR_FileSize;
}DENT;

typedef struct {
	BPB		pb;
	DENT*		root_dir_tab;
	unsigned 	root_dents;
	unsigned	root_dents_max;

	// Number free is calculated manually by scanning the FAT
	unsigned	free_clusters;
	unsigned	total_clusters;

	unsigned root_dir_sectors;
	unsigned data_sectors;
	unsigned fat_sectors;

	char		type;

	// The FAT is entirely cached in memory.
	union {
		unsigned *	fat32;
		short *		fat16;
		unsigned char *	fat12;
	};
}*P_FAT_REPR;

#define FAT32 2
#define FAT16 1
#define FAT12 0

int ReadSectorsLBA(void *b, unsigned drive, unsigned base, unsigned sectors);

unsigned ClusterInxFromPath(const char * path);

void ReadFileClusters(void *b, unsigned fat_cluster, unsigned nclusters);

unsigned ClusterIndexFromAbsFSeekBytes(unsigned seek_abs);

// FLAG: Convert to static inline and insert assertions too.
#define TIME(H, M, S)
#define DATE()

#pragma pack()

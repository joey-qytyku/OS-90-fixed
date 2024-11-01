#ifndef FAT_H
#define FAT_H

// I used the following as a reference.
// http://www.isdaman.com/alsos/protocols/fats/nowhere/FAT.HTM

#include "TYPE.H"

typedef struct __attribute__((__packed__)) {
	// Why this is 16-bit I do not understand. It is actually a shift value.
	// 512-byte sectors means 1<<10, so a value of 10 or 0xA.
	SHORT   bpbBytesPerSector;

	// Self-explanitory.
	BYTE    bpbSectorsPerCluster;

	// Number of sectors including the boot sector that are supposed to be
	// left out.
	SHORT   bpbReservedSectors;

	// The fats are simply copies. Usually there are two for redundancy
	// reasons but there can be any number.
	BYTE    bpbNumberOfFATs;

	// Number of root directory entries. On FAT12 and FAT16, this number
	// is fixed.
	SHORT   bpbRootEntries;

	// Number of sectors
	SHORT   bpbTotalSectors;
	BYTE    bpbMedia;
	SHORT   bpbSectorsPerFAT;
	SHORT   bpbSectorsPerTrack;
	SHORT   bpbHeadsPerCylinder;

	// Number of sectors that are ignored in order to find the filesystem
	// contents. This is not really the partition base and is relative to it
	// unless only one partition exists.
	LONG    bpbHiddenSectors;

	// If bpbTotalSectors is too large, this will be set to the actual
	// number of sectors. Otherwise, zero.
	LONG    bpbTotalSectorsBig;

	LONG    bpb32SectorsPerFAT;
	SHORT   bpb32Flags;
	SHORT   bpb32Version;
	LONG    bpb32RootCluster;
	SHORT   bpb32InfoSector;
	SHORT   bpb32BootBackupStart;
	BYTE    bpb32Reserved;
}BPB,*P_BPB;

// FAT32 requires an extra sector to be used for information
// The base is indicates by bpb32InfoSector.


enum {
	READ_ONLY=0x01,
	HIDDEN=0x02,
	SYSTEM=0x04,
	VOLUME_ID=0x08,
	DIRECTORY=0x10,
	ARCHIVE=0x20    // Compressed file, not sure how to handle
};
// LFN=READ_ONLY|HIDDEN|SYSTEM|VOLUME_ID

// 16-bit value
struct fat_time __attribute__((__packed__) {
	struct __attribute__((__packed__){
		BYTE    seconds:5; // /2 actually, only 2-sec precision
		BYTE    minutes:6;
		BYTE    hours:5;
	};
};

struct fat_date {
	BYTE    day:5;
	BYTE    month:4;
	BYTE    year:7;
}__attribute__((__packed__);

//
// Each directory entry is 32-byte.
//
// Each directory except for the root one has a . entry that self-references
// and a .. entry to go back. This makes it easy to get the CWD or go back.
//
typedef struct __attribute__((__packed__)){
	BYTE    name[11];
	BYTE    attr;
	BYTE    _res;

	BYTE    create_hundreths; // Added with VFAT, but still compatible.
	//
	// Next is the time it was created.
	//
	struct fat_time create_time;
	struct fat_date create_date;
	struct fat_date access_date;

	SHORT           cluster_hiword;
	struct fat_time update_time;
	struct fat_date update_date;
	SHORT           cluster_loword;
	LONG            file_bytes;
}DENT,*P_DENT;

//
// The FAT contains CHS values but we do not use them. Bochs and real BIOSes
// can have all sorts of weird problems if we trust the entries.
// Furthermore, the formatter program often does not even know these parameters,
// such as any utility for UNIX (there is no INT13H in that case)
//
// 512-byte sectors are essentially assumed here because INT 13H can only use
// 512. Most disks can emulate this, and the driver is required to do this
// anyway.
//
// Large disk support may be added later but will require manual translation of
// INT 13H on DOS.
//
typedef struct {
	BYTE    fat_type; // 0: 12, 1: 16, 2: 32
	BYTE    bios_drv; // INT 13H parameter
	SHORT   bios_sectors_per_track;
	SHORT   bios_tracks_per_cylinder;
	LONG    root_sectors;
	LONG    fat_sectors;
	LONG    data_sectors;
}FAT_FS_INFO;

#define BYTES_PER_SECTOR 512

/*
- About FAT -

It goes in this order:
	BOOT | Reserved? | FAT(s) | Root Dir | .... |

Some calculations:

// Number of sectors in the root directory
Root_Sectors = Root_Directory_Entries * 32 / Bytes_Per_Sector(or 512)

// Number of sectors in all FAT copies.
FAT_Sectors = Number_Of_FATs * Sectors_Per_FAT

// Number of sectors in the data area
Data_Sectors = Total_Sectors ? (Reserved_Sectors + FAT_Sectors + Root_Sectors)
// On FAT32, the data area is always

Total_Clusters = Data_Sectors / Sectors_Per_Cluster

FAT_Base on FAT12/16
	= bpbHiddenSectors + bpbReservedSectors
*/

/*
- Structure of the Driver -

FS Layer:

BOOL ReadClusterChain(LONG chain, PVOID buff);

BOOL ParsePathToDent(   BYTE    letter,
			PBYTE   full_path,
			PLONG   dentindex_cluster);

*/

#endif /* FAT_H */

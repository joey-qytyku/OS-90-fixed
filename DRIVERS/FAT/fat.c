#include "fat.h"

#include <assert.h>
#include <unistd.h>
#include <fcntl.h>

// memcpy?
#include <string.h>

#define DEB printf

int fd;

// NOTE: I need to handle partition offsets
int ReadSectorsLBA(void *b, unsigned drive, unsigned base, unsigned sectors)
{
	off_t o = lseek(fd, base * 512, SEEK_SET);
	read(fd, b, sectors * 512);
}

static void InitFat(
	// BIOS drive number.
	unsigned drive,
	// FAT context structure.
	FAT_REPR *f
)
{
	// Read BIOS Parameter block into temporary buffer
	// Then copy it to the structure but not as a full sector.
	// It is large enough for all types of FAT.
	// Also misalignment of the buffer is accounted for.
	{
		BPB _f;
		ReadSectorsLBA(&_f, drive, 0, 1);
		f->pb = _f;
	}

	// We need to perform anumber of calculations before we know what type
	// of FAT there is.
	// The only way to discern FAT16 and FAT12 is to count the number of
	// clusters. FAT12 may not be more than 4084 clusters by definition and
	// obviously due to the 12-bit addressing.
	//
	// FAT32 does not have a special root directory and BPB_RootEntCnt is 0.

	{
		// Some of the calculations in the spec are demonstrative and
		// not very efficient. If FAT32, the number of sectors
		// in the FAT is defined in a 32-bit value that is otherwise
		// zero.
		if (f->pb.BPB_RootEntCnt == 0) {
			DEB("Volume is FAT32");
			f->type = FAT32;
			f->fat_sectors = f->pb.f32.BPB_FATSz32;
			// ADD GOTO
			goto FoundType;
		}

		//
		f->root_dir_sectors =
		((f->pb.BPB_RootEntCnt * 32)
		+
		(f->pb.BPB_BytsPerSec - 1)) / f->pb.BPB_BytsPerSec;


	}
	FoundType:;
}

int main()
{
	fd = open("./IMGMAKE.IMG", O_RDONLY);
	assert(fd != -1);

	static FAT_REPR f;
}

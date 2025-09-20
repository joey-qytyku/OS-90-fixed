# Disk Caching Ideas

## Importance

An SSD can bring an old computer to life, and a fast filesystem can make a vintage computer fly.

## General Ideas

Disk caching can be done at many different levels with different amounts of modular separation.

- Disk block caching
- Caching FS structures (including when they are not being immediately accessed)
- File path caching
- Directory entry caching
- File data caching

Caching the disk blocks is too simplistic for OS/90 but it is possible to implement disk scheduling at the block device level. Accesses can be ordered to reduce rewinding. I think the escalator algorithm is the most reasonable.

FS structures can be cached. This includes the root directory on FAT12 or FAT16. I may want to limit this to the FAT itself and obviously the BPB or MBR.

The most important cache ideas are directory entry caching/path caching and also file caching.

## Disk scheduling

Disks have real CHS parameters, which means that files are faster to access if they are within cylinder boundaries and we avoid seeking between cylinders.

There is a conflict between how some BIOSes want to represent INT 13H CHS parameters and report them VS what the real hardware accepts, the MBR, and other problems, but the real CHS parameters do exist.

The escalator can be based on drive geometry. Consider that the read/write needle is at the same position for each platter, which contains 2 heads on most drives.

Floppies are so slow that I do not really care, but they can be single-side formatted.

Hard disks use double-sided formatting and old ones had multiple heads.

What this means is that there can be a geometric escalator algorithm. The disk has to theretically spin to complete one pass of operations on ONE head, and then do them on the next head if needed.

However, this is more of a random access optimization, which is not really the access pattern typical of these systems.

Maybe I should do more research on how magnetic storage works.

I still think caching has the largest affect on performance.

> There are also dsel optimizations. Operations on one drive can be completed before any on another, for one controller. Drive selection is actually slow. Multiple disks will not be that common though

The track alignment is actually a real thing. Partitions have to be aligned to a track or massive perfomance problems occur.

If I knew the speed of the disk and its transfer rate (some of this can be tested)

BTW the actuator arm has each RW needle together. They do not go in different directions.

> Why would it provide LBA then? Are the CHS values it takes even real?
> They would have to be.

## dentcache

dentcache can be done using a specialized hash table where the file name is hashed using CRC32. It can be done at any level, but obviously it must be done at the DOS interface level.

> I did testing and found that CRC32 is able to give no collisions for about 9500 unique file names. This is totally sufficient and better than any custom algorithm.
> It is possible to group hashes by some other data to actually allow collisions to separate

This hash table will use multi-level fixed-depth trie bucket system.

We gradually and/shift the hash requested for lookup until the reach the bottom of the list. If there a duplicates, they can be kept in a list.

I am looking into reading 5 bits at a time.

There is no need to preallocate anything, but the two-level structure goes up to 1024 entries if depth is 2. Past that point, there will be clustering, and 1024 is still not the maximum of FAT.

65536 is the maximum number of short dents in the FAT filesystem.

4^32=65536 so a depth of 4 is required to cover the physical limit of FAT. Technically, this limit is artificial, but the spec mandates it.

The pages used for dent caching can also be partially uncommitted.

> Use a pointer or index to get the real dent?

## Path caching

I think this is slightly less important because dentcache does speed up path resolution. I can avoid this.

Instead, the directory tree can be memorized and extend off of the dentcache

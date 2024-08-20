Does it need to be this complicated? The binary tree idea is cool, but it would use a lot of memory for large files and would actually be quite terrible for sequential access.

I can let the user control caching properties based on the extension though.

There are a few different types of file access:
- Sequential
- Single read into memory (e.g. executable or small TXT)
- Part of it is accessed at a time (TXT)

Those are the most common.

Typical operating systems will probably have a "block" or file data located where the file pointer is. This can be combined with

> User can match based on extensions and also the size.

# fastfat

fastfat is the file cache driver of OS/90. It conforms to a block device access and cache sharing protocol with the filesystem and disk driver.


## Cache and Block Device Protocol

Caching of disk blocks is provided by the disk driver.

## Strategy

### Root Directory and FAT

The root directory and the FAT are permanently cached in memory. A thread will occasionally write them back for the disk driver.

### Small Files

Small files (e.g. two clusters long)

### Access Counting

Every time a file is accessed with a read or write command, a number is incremented

### Request Merging

Whether a block device request goes to the disk or to the cache, request merging takes place.

> Does it need to

### Binary Tree

If the file is considered small enough, it will be placed in memory and remain resident

A binary tree is used to partition the file. Dividing the size of the file in FS blocks gives the exact size of the two branches for each node. The cluster-sized entry contains the file-local absolute cluster that must be looked up in the FAT.

An access must be resolved to one directed toward a single cluster or two if unaligned. There is a penalty for crossing the boundary.

There is a flat list of predicted translations, which all have a count tag. A total of 6 entries exist and all operations are unrolled. If the translation is used successfully, the count is incremented and the entry is exchanged with the previous if that one is lower. This is like a reverse bubble sort but with only one iteration.



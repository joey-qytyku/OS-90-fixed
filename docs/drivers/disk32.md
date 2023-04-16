# 32-bit Disk and Filesystem Access

32-bit disk access is not currently a WIP, but a possibility in the future.

All disk access goes through the BIOS by default as directed by DOS. If the filesystem is 32-bit, DOS will not call INT 13H. This means that the 32-bit FS would have to call INT 13H itself or directly interface with some kind of 32-bit disk driver.

Efficient disk access involves disk scheduling and caching data in memory. This could be done using SMARTCACHE, but that would not be the best way to do it.

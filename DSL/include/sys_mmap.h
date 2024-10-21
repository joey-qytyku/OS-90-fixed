// Idea: hold a table of the number of registers to pass
// to a sys_* function.

// All functions must match the syscall arguments.

/*
Each process has a fixed list of mappings for a file.
We can expect them
*/

/*
mmap cannot be implemented in the ideal way.

Page faults do not need to be reported by a DPMI client so memory
mapped files simply cannot work.

OS/90 may add support for userspace page faults later.

Mapping files right now simply copies it to memory. We leave it to the OS/90
or DOS extender's memory manager to handle paging if necessary.

When closed, the file data is written back.

Anonymous memory is allocated with DPMI 1.0 page allocation calls
that are supported by OS/90.

Mapping device files is not the same and currently does not work.
Files in /dev do not map.

*/
void *sys_mmap (
	void *__addr,
	size_t __len,
	int __prot,
	int __flags,
	int __fd,
	__off_t __offset);

/*
Uses DPMI locking call.
*/
int sys_mlock (const void *__addr, size_t __len);
int sys_munlock (const void *__addr, size_t __len);

/*
All memory allocated for a process is unlocked.
*/
int sys_munlockall (void);

/*

*/
int sys_mprotect (void *__addr, size_t __len, int __prot);
int sys_mprotect (void *__addr, size_t __len, int __prot);

void *mremap (
	void *__addr,
	size_t __old_len,
	size_t __new_len,
	int __flags,
	void *new_addre);

//
// This is not considered deprecated.
// May not need to implement.
//
int sys_remap_file_pages(
	void *__start,
	size_t __size,
	int __prot,
	size_t __pgoff,
	int __flags);

//
// Writes back all data back to the file.
//
int sys_msync(void addr[.length], size_t length, int flags);

int sys_shm_open (const char *__name, int __oflag, mode_t __mode);
int sys_shm_unlink (const char *__name);

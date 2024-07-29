OS/90 has an API for loading native executables into memory and performing all required operations to prepare them for execution. It works for any subsystem.

The following structure defines the functions that must be implemented.
```c
FNPTR(LONG, EXEC_FILE_OPENER,   (PBYTE path) );
FNPTR(LONG, EXEC_FILE_CLOSER,   (LONG handle) );
FNPTR(LONG, EXEC_FILE_SEEK,     (LONG handle, LONG seek) );
FNPTR(LONG, EXEC_FILE_READ,     (LONG handle, PVOID buff, LONG size) );

typedef struct {
	EXEC_FILE_OPENER        fopen;
	EXEC_FILE_CLOSER        fclose;
	EXEC_FILE_SEEK          fseek;
	EXEC_FILE_READ          fread;

	EXEC_FILE_PTRTAB        ptrtab;
}EXEC_LOADER;
```

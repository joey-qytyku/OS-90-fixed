#include <stdint.h>

#define BYTE uint8_t
#define SHORT uint16_t
#define LONG  uint32_t

#define PBYTE BYTE*
#define PSHORT SHORT*
#define PLONG LONG*

#define STATUS LONG

// Must be 16 bytes
typedef struct {
	LONG    sentinel1;
	SHORT   locks;
	BYTE    paras;
	LONG    sentinel2;
}BLOCK;
// What about the master pointer table?

PVOID M_HAlloc(LONG bytes);

VOID M_HFree(PVOID handle);

M_HLock(PVOID handle);

M_HUnlock(PVOID handle);

LONG M_HTotalAvail(VOID);

LONG M_HGetSize(LONG);

STAT M_HValidate(LONG);

STAT M_HInit(LONG heap_size)
{
}

int main()
{}

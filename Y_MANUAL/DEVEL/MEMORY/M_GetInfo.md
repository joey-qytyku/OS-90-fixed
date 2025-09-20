# M_GetInfo
```c
LONG M_GetInfo(BYTE type)
```

|Index|Meaning|
------|-------------------------------------------------------------------------
  0   | Free extended memory pages
  1   | Size of extended memory in pages
  2   | Free disk backed space in pages (DBS)
  3   | Size of DBS in pages
  4   | Address space pages free
  5   | Address space pages total
  6   | Largest range that can be allocated of mapped or uncommitted pages
  7   | Free conventional memory
  8   | Largest conventional memory block
  9   | Percentage of extended memory pages locked (0-100 integer)
  10  | Previous page faults per second value

For maximum accuracy, disable preemption, especially when performing a multi-step calculation with results. This is safe.

	Example
	~~~~~~~
................................................................................
LONG MemoryInUsePercent()
{
	// Disable
	S_PreemptInc();
	LONG v = (M_GetInfo(0)*100) / (M_GetInfo(1)*100);
	// Enable
	S_PreemptDec();
}
................................................................................

The conventional memory is not required to be 640K in size. Use the appropriate BIOS function for this. The EBDA uses part of this space and is at least 1K in size if it exist at all.

The LPT4 port base in the BDA is replaced with the EBDA segment on PS/2 and above. The first word of this segment is the size of the EBDA in KB.

Additionally, there are BIOS options to limit CM to 512K or possibly less. While not idea, this is possible. Use the BIOS for this.

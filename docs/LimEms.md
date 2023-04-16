# LIM EMS

LIM EMS cannot be used as regular memory because it uses page-based bank switching and is incompatilble with the memory manager. EMS card can, however be used as a ramdisk or swap.

This would require bypassing the Limulator built into OS/90 to make calls to the actual driver.

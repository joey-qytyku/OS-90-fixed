# Executable Compression Whitepaper

Work in progress.

Most executable compressors for DOS have mediocre compression ratios because they use run length encoding. This whitepaper describes an executable compression method that is designed specifically to compact code.

The algorithm essentially uses a compressed instruction set of its own that is more dense than 80x86 to compress 32-bit code but not entirely reduce load times.

# Command Set

Commands are at nibble boundaries. Opcodes are one nibble but if the have the code `F` an extension nibble is used.

The code is always regenerated exactly as it was before compression.

The following is the base set:
```
n[0]
Generate near RET
Set register of next nibble to one
xor r32,r32 GPR to zero, n[1] is reg mask

Repeat zero byte n[1] times
```


Ideas:
- Zero memory location
- How do I compress SIB/ModRM instructions? They seem to take up the most space.

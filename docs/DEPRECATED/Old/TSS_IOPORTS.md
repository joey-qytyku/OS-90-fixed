# TSS and IO Permissions

OS/90 supports emulation of IO instructions for userspace software, but when the kernel needs to run virtual 8086 mode to perform system tasks, IO must be directly sent to hardware. There are two ways of doing this.

IOPL is going to be 3 for all tasks no matter what. Even though the CPL is less than or equal to the IOPL, it is still necessary to have an IOPB with all zeroes (allow) or there will be a fault and the instruction will not be directly executed. The IOPB is 8192 bytes long, but gives real mode code a performance boost because instructions run directly. Or so it seems.

The other way to do it is to emulate all port IO. Decoding the instructions involves checking bits in the opcode that reveal different attributes, such as direction, imm8 or DX, or string/no string. These bits are not documented, but have predictable meanings and make decoding nothing more than a few condition checks and bit masks.

Directing them to real IO operations is a different story. We decode because emulating every possible instruction sequence would not be dense enough.

## Decision

The TSS may be a very large structure with the IOPB, but we will use it to simplify the IO virtualization functionality. Interpreting the IO instructions would cause the system to crawl during IO. 8K of memory is the cost of better performance.

But IO opcodes are faster in ring-0 protected mode because they do not need to access the IOPB or check IOPL. String instructions justify emulation overhead. Protected mode IO is apparently faster than real mode.

The boost is significant and proven by the 80386 manual. When in virtual 8086 mode, INS iteration is 29 clocks and an OUTS is 28. If CPL <= IOPL, like with the ring 0 kernel, the IO takes a mere 8 clocks. That is potentially three times more performance per instruction minus the overhead of entering ring 0. For disk access involving many reads and writes, this is massive.

For example, reading a 512-byte sector, which is 256 IO operations, will take *approximately* 100 + 2048 clocks, with a hundred or more for the emulation overhead. If we did this with virtual 8086 mode, 7168 clocks would be used.

Setting the emulation policy is not possible because IOPL is always three and a zero allow bit in the IOPB would cause ring 3 IO to actually take place.

> Long story short, IO is __fully__ emulated under OS/90. This is done within a __non-preemptible context__.

## Implementation

A boolean will decide if the emulation is direct or indirect. dosvm.md explains how indirect emulaton is done for virtual peripherals.

Direct emulation is implemented in assembly with a series of branches. The first two check for `rep insw` and `rep outsw`. The rest check for the following opcodes and run them:

```
INB imm8    E4 xx
INW imm8    E5 xx
OUTB imm8   E6 xx
INB DX      EC
INW DX      ED
OUTB DX     EE
INSB        6C
INSW        6D
OUTSB       6E
OUTSW       6F
```

Single operations using imm8 will run as the DX form.

> 32-bit IO is currently not supported under V86. String operations are also assumed to be with DF=0

## Virtual Device Drivers

### MMIO Emulation

Memory mapped IO emulation is NOT supported unless it is for simple RAM-like regions such as framebuffers. Emulating something like SATA or any other thing is not possible.

### On-board VDDs

The PIC is emulated as far as ensuring that non-specific EOIs get sent and discarded, as they are not actually used for anything.

VMM32 had a wierd way of redirecting interrupts to a virtual machine, and that was the only way to service interrupts up until Windows 3.0 when "bimodal interrupts" were added. It used some kind of "virtual PIC" driver.


### 80x86 IO Port Operations

It is not documented, but the ISA can be reverse engineered by looking at the opcode bits. The true format of the universal IO instruction is the following:
```
// Layout of an IO opcode:
// |S|1|1|0|V|1|D|W|
//
// Bits if one:
// V: Variable (uses DX)
// S: String
// D: Output
// W: Word
//```
This information is used to convert x86 instructions into a packet to be used for virtual drivers.

Here is a list of all opcodes, excluding rep (F0) prefixes.
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

For fast emulation in SV86, rep prefixes are checked. If there is not one, it may still be a string op. 32-bit IO is a different thing which is implemented less optimally if there is a 66h prefix, which must come after REP (all assemblers do this, including MASM, FASM and NASM).



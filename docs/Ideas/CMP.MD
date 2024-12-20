> While this can be useful for OS/90, I have more to gain from compressing 64-bit code. The relocations are not relevant because those are applied after loading.

# Codes

> This is intended to be mostly single pass or something.

00-0F   Repeat next byte CODE & 0xF times.

(TODO)
10-2F   Encode raw bytes.

30 ZZ   Encode ZZ zeroes.

31      Encode 2 zeroes
32      Encode 3 zeroes
32      Encode 4 zeroes

3_ NN   Encode NN nops

__ RR   Push registers bit mask, order A,B,C,D,SI,DI,BP
__ XX   Insert address from address table entry XX.
__


Important x86 instructions to leave alone:
- C3 (ret)
- Make a list of undefined opcodes and use those! Use SDM for up-to-date info.
- We cannot rely on this because data will likely have undefined codes. There needs to be an explicit encode command.

# Algorithm

The algorithm requires multiple passes to work correctly and efficiently.

The order of generation is very important and follows:
- Address table entry insertions
- Multiple consecutive register pushes
- NOP encoding (anything more than two)
- Zero encoding

# Examples

```
	push    40
	push    msg
	call    printf
	add     esp,8

msg:    DB ""

```
For 32-bit elf, this will be something like this:

```
0:  6a 28                   push   0x28
2:  ff 35 10 00 00 00       push   DWORD PTR ds:0x10
8:  e8 03 00 00 00          call   10 <msg>
d:  83 c4 08                add    esp,0x8
```

Normally, the near call has no operands and has 4 zero bytes. This can be reduced, along with other things. An estimated two bytes may be saved for a small program on the pointer.

The savings are about 6 bytes.

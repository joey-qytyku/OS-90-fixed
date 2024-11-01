> While this can be usefull for OS/90, I have more to gain from compressing 64-bit code. The relocations are not relevant because those are applied after loading.

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

__ RR   Push registers, order EAX, EBX, ECX, EDX, ESI, EDI.

__ XX   Insert address from address table entry XX.

__


Important x86 instructions to leave alone:
- C3 (ret)

# Algorithm

The algorithm requires multiple passes to work correctly and efficiently.

The order of generation is very important and follows:
- Address table entry insertions
- Multiple consecutive register pushes
- NOP encoding (anything more than two)
- Zero encoding (in order of count up to N no)

# If I Designed x86

I will remove the following opcodes:
* INTO: Why would anyone ever use this?
* INT3: Just use INT 3!
* XLAT: Why waste microcode on this?

ARM-style predication prefix byte for all instructions. It could be used to implement conditional procedure calls and near jumps.

```
mov.z ax,10
call.ge GreaterOrEqual
```

Extended INT instruction that also sets the AH register before going in.

```
mov     dx,MyString
int     21h,9h
```

This could be implemented in software, but now there is an official opcode for it.

Move immediate to segment register:
```
mov es,0B800h
```

REPZ and REPNZ can be applied to more than just string instructions.

```
mov cx,10
rep call CallMePlz
```

Or to slow the CPU:
```
mov cx,65535
rep nop
```

Push/pop multiple registers with bit mask parameter:
```
push ax,bx
pop  ax,bx
```

Remove all addressing modes with two registers and make all registers capable of memory access.

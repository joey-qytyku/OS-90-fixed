# Address Space Extensions

OS/90 implements a layer above all standard INT calls that take memory operands and fully implements the Microsoft DPMI extensions.

What this means is that protected mode programs in any mode can pass protected mode segment selectors and addresses (32-bit or 16-bit) and all functions specified to work should do so.

The kernel has its own way of doing the basic translation for INT 21h where the same registers are used by most functions with few exceptions.

Some other APIs such as INT 13H must also run in protected mode because True DPMI requires this so that formatting disks can be done from the Windows explorer.

Aside from the APIs specified to be extended by the kernel, there is a set of API calls that is designed to assist in writing lightweight hooks.

## The types of Extensions

32-bit kernel software sometimes intends to fully capture an interrupt function code, or the whole thing in rare cases and it needs to make sense of pointers passed to it.

The other type of extending is the conversion of operands to real mode and then sending them down the chain or terminating. This is what the OS does for real mode compatibility. Not advised for drivers.

## INT 13H ASE (NEEDS UPDATES)

These are PARTIALLY supported by both Microsoft and by HDPMI. OS/90 implements ASE for the newer extended functions as well as the basic ones.

There is no abstraction on drive letters. They mean only what they mean to the BIOS and nothing else.

The implementation of this accounts for the fact that transfers may require the use of DMA which only operates on physical ranges.

ASE for INT 13H is provided by the kernel. This allows the use of a 32-bit FS without using any 32-bit disk drivers.

## INT 21H ASE

Most INT 21H functions use the same registers to send addresses, but NOT all.

A specialized lookup table is used to indicate a "conformant function which takes a memory operand" or one that takes it at a non-standard register.

Practically all pointers in the DOS API are far pointers that use DS.

Some function codes are fully or partially hooked however, and in this case, translation is handled a bit differently, as we are trying to access data from PM that could come from 16/32-bit PM or real mode.

> Can I use function pointers in the PCB that extend registers? Not quite...
>

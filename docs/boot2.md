# Boot process and Legacy Drivers

## GRABIRQ.SYS

This is a device driver loaded by the DOS kernel on startup. It traps INT 21H calls and records modifications to the real mode interrupt vector table using the DOS API calls 25H and 35H. This is so that IRQ lines assumed to be free can be used by devices and busses with plug-and-play support (ISA PNP/PCI).

This driver is required and its installation is checked on boot.

## OS90.COM

This program loads the kernel and communicates some information that is easier to get in real mode.

# HMA Issues

The HMA is used for startup and later on for ISA DMA. XMS 2.0 lets only one program can use it at a time, and in this case that is the 32-bit kernel. Remove DOS=HIGH from confg.sys and anything that may use it. All software is required to have a no-HMA option.

# DOS drivers

16-bit drivers will work. The DOS .SYS driver model is unsupported by the 32-bit kernel, but the DOS kernel and programs can still use them within a DOS VM. TSR programs like MS Mouse will also function as expected as the mouse interrupt is expected to be 16-bit. Plug-and-play ISA card drivers work even if the ISAPNP driver is not installed. These utilities may do various things with the cards, but will certainly not manipulate card select numbers set by the BIOS. PnP configuration utilities may do this.

The ISAPNP bus driver scans the cards on the system for their reosurce assignments. If a card uses interrupts identified as 16-bit, the interrupt resources cannot be modified because the 16-bit driver will not expect that. Otherwise, resources are changed. The isolation protocol is not performed by ISAPNP and CSNs are preserved.

In short, ISA plug-and-play cards can operate with 16-bit DOS drivers just fine.

# Parameters

The PSP of the bootloader is passed to the kernel for parsing. They are case insensitive.

# Architecture

## The Kernel

The OS/90 kernel and associated drivers are an array of potatoes wired together to form a circuit. The highly modular design allows parts to be swapped in and out with good garauntees of proper behavior.

The OS/90 kernel is monolithic and modular. It is designed to handle every high-level feature used by drivers in a way as close to the hardware as possible. For example, features in the memory manager allow for automatic tiered cache management, mapping of disk sectors into the address space, and hooking of page faults for emulation of MMIO.

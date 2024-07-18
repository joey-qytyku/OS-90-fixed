# Virtual Machines

OS/90 will support semi-accelerated virtual machines.

# Emulating Real mode

Virtual 8086 mode can be used for the VM, though it does require extensive use of task scheduling hooks. It also cannot simulate the HMA and attempting to access it will crash the system.

Exceptions are caught in the necessary way to produce the correct emulation effects ofr various instructions. IO operations are virtualized.

# Emulating Paging

The memory management unit must be simulated using page hooking. This significantly degrades performance but is unavoidable and is still better than dynamic translation.

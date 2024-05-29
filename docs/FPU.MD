# Floating Point Unit

OS/90 supports the 80387 FPU and better. The kernel deals with everything related to the FPU.


## Task Switching

Multiple tasks can use the FPU. OS/90 uses lazy switching.
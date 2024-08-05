# Virtual DOS Policy

DOS programs are known to use the available memory very aggressively because they assume a single-tasking environment. OS/90 has features to reduce memory usage by underreporting resource availability.

Direct hardware access is also frequently done.

OS/90 permits the adjustment of the following resource allocations:
- Virtual devices at a per-VM basis
- Conventional memory reporting
- Initial program allocation (for .COM executables)
- Extended memory
- Virtual address space reported by DPMI

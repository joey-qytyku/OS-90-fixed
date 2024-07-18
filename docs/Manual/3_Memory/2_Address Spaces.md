# Address Spaces Explained

## Real Mode Addressable Memory

OS/90 permits the modification of page table entries belonging to the 1MB region, and this can be done per-process inside the scheduler hook. This permits the DOS subsystem to partially simulate the DOS memory and allow programs to access memory that normally would not be available.

The mappings of the real mode memory are by default identity-mapped. No mapping function can change the mappings. Any attmept to change them outside the scheduler hook should be done with interrupts fully disabled.

> The HMA is completely non-accessible. DOS programs just cannot use it.

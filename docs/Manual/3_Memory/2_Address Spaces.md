# Address Spaces Explained

## Real Mode Addressable Memory

OS/90 permits the modification of page table entries belonging to the 1MB region, and this can be done per-process inside the scheduler hook.

The mappings of the real mode memory are by default identity-mapped. No mapping function can change the mappings. Any attmept to change them outside the scheduler hook should be done with interrupts fully disabled.

The page table entries for the first 1MB are permitted.

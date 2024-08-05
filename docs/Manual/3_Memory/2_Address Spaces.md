## Address Spaces Explained

### Real Mode Addressable Memory

Real mode addressable memory is 100% global and mappings never change. This means that using 16-bit services does not require any address space translation or other things.

The HMA is completely non-accessible. DOS programs just cannot use it, and nothing is permitted to touch its physical or even virtual address space. The kernel depends on it being identity-mapped, and the pages are ring-0.

### CHANGELONG

August 4 2024: Isolated DOS memory is removed. Too complicated and not much benefit.

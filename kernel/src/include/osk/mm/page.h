#ifndef PAGE_H
#define PAGE_H

#define MB(x) ((x) * 1048576)
#define KB(x) ((x) * 1024)

#define PDE_SHIFT 22
#define PTE_SHIFT 12

// What?
#define PDE_MASK 0b1111111111
#define PTE_MASK 0b1111111111

#define PAGE_SIZE (4096)

#define PG_D     (1<<6) /* Page is dirty, was accessed */
#define PG_A     (1<<5) /* The page table/dir was used */
#define PG_PCD   (1<<4) /* Page cache disable */
#define PG_PWT   (1<<3) /* Page write though cache */
#define PG_U     (1<<2) /* User */
#define PG_RW    (1<<1) /* Read/write */
#define PG_P     (1<<0) /* Present */

// The unused bits are used for OS/90 to implement uncommitted memory and
// locking. These only apply to page table entries.

// AVL is 3-bit. A number is used to indicate these mutually-exclusive
// properties.

// OS/90: Memory can be swapped
#define PTE_TRANSIENT (0b001<<9)

// OS/90: Page part of uncomitted block
#define PTE_UCM       (0b010<<9)

// OS/90: Flag indicating page mapping free. Page is not present and accessing
// is an error.
#define PTE_UNUSED    (0b100<<9)

#endif /* PAGE_H */

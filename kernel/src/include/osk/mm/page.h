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

//
// Memory can be swapped. By default, memory is locked.
// This is only applied to pages that are present and allocated.
//
#define PTE_TRANSIENT (0b001<<9)

//
// Page part of uncomitted block (Awaiting commit)
// Actual page does not exist and cannot be applied to allocated page.
// Doing so may cause an error.
//
#define PTE_AWC       (0b010<<9)

//
// Flag indicating page mapping free. Page is not present
// and accessing is a fatal error for kernel and fault for user.
//
#define PTE_UNUSED    (0b011<<9)

//
// Page is hooked. This means that a page fault should be broadcasted to
// the kernel-mode handler of the current task.
//
// This is a modifier to a non-present page. If the page is present, this
// means nothing. It is used to implement IO memory region emulation,
// especially for framebuffers.
//
#define PTE_HOOK      (0b100<<9)

//
// Page is transient (swappable) and is currently on the disk.
// The address field of the PTE MUST be the page granular offset on the disk.
//
// USING THIS ANYWHERE OUTSIDE OF THE MEMORY MANAGER CODE IS ALWAYS WRONG.
//
#define PTE_TRANSIENT_OUT (0b101<<9)

#endif /* PAGE_H */

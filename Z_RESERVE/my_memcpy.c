#include <stdio.h>

// Not currently in use.

#define rep_stosd(d,v,c) \
__asm__ volatile ( \
	"movl %0,%%edi\n\t" \
	"movl %1,%%eax\n\t" \
	"movl %2,%%ecx\n\t" \
	"rep stosl" \
	::"rmi"(d),"rmi"(v),"rmi"(c) \
	:"memory","eax","ebx","ecx","edi" \
)

#define _copy4(d,s,c)\
__asm__ volatile ( \
    "movl %0,%%edi\n\t"\
    "movl %1,%%esi\n\t"\
    "movl %2,%%ecx\n\t"\
    "rep movsl"::"rmi"(d), "rmi"(s), "rmi"(c)\
    :"memory", "edi", "esi", "ecx"\
)\

#define _copy1(d,s,c) \
__asm__ volatile ( \
    "movl %0,%%edi\n\t"\
    "movl %1,%%esi\n\t"\
    "movl %2,%%ecx\n\t"\
    "rep movsb"::"rmi"(d), "rmi"(s), "rmi"(c)\
    :"memory", "edi", "esi", "ecx"\
)\

// This may fail with arbitrary sizes.
static inline void _copy_large(void * __restrict dest, const void *__restrict src, unsigned count)
{
    // These represent the unaligned byes at the start.
    // Unaligned bytes at the end are 4-x_ua
    // Sure it works like that? Maybe depends on the count.
    unsigned dest_uabytes = 0;
    unsigned src_uabytes = 0;
    if (((unsigned)(dest) & 0b11) != 0) {
        // Destination is misaligned. We will align the pointer and perform
        // a separate copy of the non-aligned data.
        // This will cause (4-uabytes) to be part of the total
        // unaligned bytes.
        dest_uabytes = 4 - ((unsigned)(dest) & 3);
    }

    if (((unsigned)(src) & 3) != 0) {
        src_uabytes = 4 - ((unsigned)(src) & 3);
    }
    _copy4(     ((unsigned)dest + 3) & (~3),
                ((unsigned)src + 3)  & (~3),
                (count - dest_uabytes - src_uabytes)>>2);


    // Copy the extraneous head bytes.
    // _copy1(dest, src, src_uabytes);

    // Tail bytes copy.
    // _copy1(dest+(count&(~3)), src+(count&(~3)), ((count - 4 ) & 0b11) );
}

void *_memcpy(void * __restrict dest, const void *__restrict src, unsigned count)
{
    if (count > 16) {
        _copy_large(dest, src, count);
    }
    return dest;
}

char s[] = "Important message: hello world                                                  ";

# TODO

General TODO:
- Everything in assembly needs to be rewritten.
- What if a function is hooked while it is running? How do I deal with that?
What if a function is hooked while it is running? How do I deal with that?- Type.h changes
- Change stack size to 3K instead of 2K. It is good to have more.
- Support virtualizing the DOS memory. Swapping the first page directory with a single write is necessary (and a special page table for it too). This is a trick used by VMM32 to avoid 160+ memory transfers. This also means that DOS VMs will have an added overhead of 4K.

Keep in Mind:
- Write things in C before trying to assemblify

Remember that it may be better to use resizable arrays rather than linked lists. A resizable array will have faster lookup times in all cases.

Linked lists can be made faster by knowing the size and the middle of the list, cutting the worst case in half.

## Long Term Strategy

- Set up source code
- Get SV86 working and TEST IT

> Major and minor function code for things? For example major is "read" and minor says something about buffering?

- How do I do this? <https://en.wikipedia.org/wiki/SUBST>

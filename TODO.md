# TODO

General TODO:
- Everything in assembly needs to be rewritten.
- Type.h changes
- Flatten includes
- Do my error codes need to be so complicated?
- Change stack size to 3K instead of 2K. It is good to have more.
- Support virtualizing the DOS memory. Swapping the first page directory with a single write is necessary (and a special page table for it too). This is a trick used by VMM32 to avoid 160+ memory transfers. This also means that DOS VMs will have an added overhead of 4K.

Keep in Mind:
- Write things in C before trying to assemblify

To get things done, I need a scheduler. Within the next two weeks, I want the full transition to DMC to be complete.

Remember that it may be better to use resizable arrays rather than linked lists. A resizable array will have faster lookup times in all cases.

Linked lists can be made faster by knowing the size and the middle of the list, cutting the worst case in half.

## Long Term Strategy

- Set up source code
- Get SV86 working and TEST IT

The scheduler/IRQ stuff is basically a one-shot deal. It all has to work at once. I can of course mask other interrupts if I do not want to test RMCA, but there are a lot of connected components for both.

- Measure work completed, not time spent

> Major and minor function code for things? For example major is "read" and minor says something about buffering?

- How do I do this? <https://en.wikipedia.org/wiki/SUBST>

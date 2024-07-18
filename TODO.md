# TODO

Keep in Mind:
- Write things in C before trying to assemblify

General TODO:
- Everything in assembly needs to be rewritten.
- Type.h changes
- Flatten includes
- Do my error codes need to be so complicated?

To get things done, I need a scheduler. Within the next two weeks, I want the full transition to DMC to be complete.

Remember that it may be better to use resizable arrays rather than linked lists. A resizable array will have faster lookup times in all cases.

## Long Term Strategy

- Set up source code
- Get SV86 working and TEST IT

The scheduler/IRQ stuff is basically a one-shot deal. It all has to work at once. I can of course mask other interrupts if I do not want to test RMCA, but there are a lot of connected components for both.

- Measure stuff completed, not time spent

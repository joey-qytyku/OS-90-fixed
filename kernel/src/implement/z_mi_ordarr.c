/*******************************************************************************
		      Copyright (C) 2022-2024, Joey Qytyku

  This file is part of OS/90.

  OS/90 is free software. You may distribute and/or modify it under
  the terms of the GNU General Public License as published by the
  Free Software Foundation, either version two of the license or a later
  version if you choose.

  A copy of this license should be included with OS/90.
  If not, it can be found at <https://www.gnu.org/licenses/>
*******************************************************************************/

typedef struct {
    unsigned int value;
    void* prev;
    void* next;
}LIST_ITEM, *PLIST_ITEM;

static PLIST_ITEM FindIndexDescending(PLIST_ITEM ll_root, unsigned int count)
{
    PLIST_ITEM c = ll_root;
    for (int i = 0; i < count; i++) {
        c = c->prev;
    }

    return c;
}

//
// Returns a pointer to the list item that needs to have the value inserted
// before it in order to maintain order.
//
// Detecting if the item should be at the very end or at the begining
// only matters for the edge case of the begining or last item of the full
// list, not subsets of it.
//
// A simple non-recursive wrapper is needed to make this function work.
//
PLIST_ITEM GetInsertionPoint(PLIST_ITEM ll_root, unsigned int value, unsigned int set_size)
{
    if (set_size == 1) {
        return ll_root;
    }
    else {
        unsigned int bottom_size = set_size / 2;
        unsigned int top_size = set_size - bottom_size;

        // If the bottom size is zero after the division, round it to one
        // and ensure that top_size get one more to compensate.
        if (bottom_size == 0) {
            bottom_size = 1;
            top_size++;
        }

        PLIST_ITEM bottom_root = FindIndexDescending(ll_root, top_size-1);
        PLIST_ITEM top_root    = ll_root;

        // printf("bs: %i ts: %i val: %i\n", bottom_size, top_size, ll_root->value);
        // printf("brv: %i trv: %i \n-------------\n", bottom_root->value, top_root->value);

        // If the value being inserted is larger than the bottom half, we
        // are sure that the bottom does not need to be searched at all.
        // It does not belong there anyway.
        if (value > bottom_root->value) {
            // Perform process again for reduced set
            return GetInsertionPoint(top_root, value, top_size);
        }
        else {
            // Otherwise, the value is less than or equal to the highest in the
            // bottom half of the set
            // We will perform the process above but for the bottom of the set
            return GetInsertionPoint(bottom_root, value, bottom_size);
        }
    }
}

// I think there needs to be a not found case, or at least handling
// for null pointers.

//
// Maybe check if greater than highest or lower value in set.
//
// Or I can require nonsense values to be inserted that should be ignored.
//

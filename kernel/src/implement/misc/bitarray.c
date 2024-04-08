#include <stdbool.h>

#define BIT_IS_SET(num,bit) ((num & (1<<bit))>0)

void enable_bit_array_entry(Int *array, Int inx)
{
    Int bit_offset  = inx & 31;
    Int Int_index = inx >> 5;

    array[Int_index] |= 1 << bit_offset;
}

void disable_bit_array_entry(Int *array, Int inx)
{
    Int bit_offset  = inx & 31;
    Int Int_index = inx >> 5;

    array[Int_index] &= ~(1 << bit_offset);
}

__attribute__((hot))
inline bool get_bit_array_entry(Int *array, Int inx)
{
    Int bit_offset  = inx & 31;
    Int Int_index = inx / 32;

    return BIT_IS_SET(array[Int_index], bit_offset);
}

void enable_bit_array_range(pmInt array, Int base_inx, Int count)
{
    for (Int i = 0; i < count; i++)
        enable_bit_array_entry(array, base_inx+i);
}

__attribute__((hot))
Int alloc_bits(
    pmInt  array,
    Int    bound,
    Int    num
){
    Int i = 0;
    Int j = 0;
    Int counter = 0;

    if (num == 0) {
        return -1;
    }

    while (1) {
        j = 0;
        if (i >= bound)
            goto fail;
        if (get_bit_array_entry(array, i) == 0) {
            j = i;
            counter = num;
            while (1) {
                if (get_bit_array_entry(array, j) == 0) {
                    j++;
                    if (j >= bound) {
                        goto fail;
                    }
                    counter--;
                    if (counter == 0)
                        goto success;
                }
                else {
                    goto restart_outer;
                }
            }
        }
        restart_outer:
        i = j + 1;
    }
    success:
    enable_bit_array_range(array, i, num);
    return i;

    fail:
    return -1;
}

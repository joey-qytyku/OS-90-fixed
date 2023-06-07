#ifndef LOG2_H
#define LOG2_H

// A simplified version of the one I found on stack overflow.
// This is useful for bit field structure members. It is NOT the same as
// log2.

#define REPRESENT_IN_X_BITS(n, x)\
(\
    ( ((1U<<x)-1U) >= (n) ) ? 1 : 0\
)

#define BITS_NEEDED(number)\
(\
    (number) == 1 || (number) == 0 ? 1 : \
    REPRESENT_IN_X_BITS(number, 1) ? 1 :\
    REPRESENT_IN_X_BITS(number, 2) ? 2 :\
    REPRESENT_IN_X_BITS(number, 3) ? 3 :\
    REPRESENT_IN_X_BITS(number, 4) ? 4 :\
    REPRESENT_IN_X_BITS(number, 5) ? 5 :\
    REPRESENT_IN_X_BITS(number, 6) ? 6 :\
    REPRESENT_IN_X_BITS(number, 7) ? 7 :\
    REPRESENT_IN_X_BITS(number, 8) ? 8 :\
    REPRESENT_IN_X_BITS(number, 9) ? 9 :\
    REPRESENT_IN_X_BITS(number, 10) ? 10 :\
    REPRESENT_IN_X_BITS(number, 11) ? 11 :\
    REPRESENT_IN_X_BITS(number, 12) ? 12 :\
    REPRESENT_IN_X_BITS(number, 13) ? 13 :\
    REPRESENT_IN_X_BITS(number, 14) ? 14 :\
    REPRESENT_IN_X_BITS(number, 15) ? 15 :\
    REPRESENT_IN_X_BITS(number, 16) ? 16 :\
    REPRESENT_IN_X_BITS(number, 17) ? 17 :\
    REPRESENT_IN_X_BITS(number, 18) ? 18 :\
    REPRESENT_IN_X_BITS(number, 19) ? 19 :\
    REPRESENT_IN_X_BITS(number, 20) ? 20 :\
    REPRESENT_IN_X_BITS(number, 21) ? 21 :\
    REPRESENT_IN_X_BITS(number, 22) ? 22 :\
    REPRESENT_IN_X_BITS(number, 23) ? 23 :\
    REPRESENT_IN_X_BITS(number, 24) ? 24 :\
    REPRESENT_IN_X_BITS(number, 25) ? 25 :\
    REPRESENT_IN_X_BITS(number, 26) ? 26 :\
    REPRESENT_IN_X_BITS(number, 27) ? 27 :\
    REPRESENT_IN_X_BITS(number, 28) ? 28 :\
    REPRESENT_IN_X_BITS(number, 29) ? 29 :\
    REPRESENT_IN_X_BITS(number, 30) ? 30 :\
    REPRESENT_IN_X_BITS(number, 31) ? 31 : 32\
)

// First we check if the number is zero or one. In either case, we will need
// one bit to represent them.
// Then we check if it can be represented with that number of bits and return
// that exact number if so.
//
// If it could not be represented with 32 bits, it is assumed to be 32
// since we have exhausted all other possibilities.
//

#endif /* LOG2_H */

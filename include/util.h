#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

/**
 * Get next power of 2:
 *
 * Arguments:
 *  @n: integer
 *
 * Returns:
 *  @success: next power of 2
 *  @failure: does not
 */
static inline size_t
next_pow2(size_t n)
{
        if (n == 0)
                return 1;

        n--;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;

        if (sizeof(n) >= 2)
                n |= n >> 8;
        if (sizeof(n) >= 4)
                n |= n >> 16;
        if (sizeof(n) == 8)
                n |= n >> 32;

        return n + 1;
}

#endif /* #ifndef UTIL_H */

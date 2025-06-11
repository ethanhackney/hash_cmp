#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>
#include <stdint.h>

/* hash map size type */
typedef uint64_t hash_map_size_t;

/**
 * mark condition as unlikely:
 *
 * Arguments:
 *  @_cond: condition
 *
 * Returns:
 *  @true:  if condition was true
 *  @false: if not
 */
#define unlikely(_cond) \
        __builtin_expect(!!(_cond), 0)

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
static inline hash_map_size_t
next_pow2(hash_map_size_t n)
{
        if (n == 0)
                return 1;

        n--;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        n |= n >> 32;
        n++;

        return n;
}

#endif /* #ifndef UTIL_H */

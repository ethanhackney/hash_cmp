#define PLD_HASH_MAP_INIT_CAP 1
#include "include/pld_hash_map.h"
#include <stdio.h>
#include <assert.h>

static inline hash_map_size_t
inthash(int i)
{
        hash_map_size_t hash = (hash_map_size_t)i;

        hash ^= hash >> 15;
        hash ^= hash >> 7;
        hash ^= hash >> 3;
        hash ^= hash << 5;
        hash ^= hash >> 16;

        return hash;
}

#define intcmp(_a, _b) (((_a) > (_b)) - ((_a) < (_b)))

PLD_HASH_MAP_DEFINE(int, int, int2intmap, inthash, intcmp)

int
main(void)
{
        int log2[] = {
                -1, /* nil */
                 0, /* 1 */
                 1, /* 2 */
                 2, /* 4 */
                 3, /* 8 */
                 4, /* 16 */
                -1, /* nil */
        };
        struct int2intmap *i2imap = NULL;
        int *vp = NULL;
        int i = 0;

        i2imap = int2intmap_new(0);
        if (i2imap == NULL)
                puts("here");

        for (i = 1; log2[i] >= 0; i++)
                assert(int2intmap_set(&i2imap, i, log2[i]) == 0);

        for (i = 1; log2[i] >= 0; i++) {
                vp = int2intmap_get(i2imap, i);
                assert(vp != NULL);
                assert(*vp == log2[i]);
        }

        for (i = 1; log2[i] >= 0; i++)
                assert(int2intmap_unset(&i2imap, i) == 0);

        int2intmap_free(&i2imap);
}

#include "include/pld_hash_map.h"
#include <stdio.h>

#define inthash(_n) ((size_t)(_n))
#define intcmp(_a, _b) (((_a) > (_b)) - ((_a) < (_b)))

PLD_HASH_MAP_DEFINE(int, int, int2intmap, inthash, intcmp)

int
main(void)
{
        struct int2intmap *i2imap = NULL;
        int *vp = NULL;

        i2imap = int2intmap_new(0);
        if (i2imap == NULL)
                puts("here");

        if (int2intmap_set(&i2imap, 16, 4) < 0)
                puts("here");

        vp = int2intmap_get(i2imap, 16);
        if (vp != NULL)
                printf("%d\n", *vp);

        if (int2intmap_unset(&i2imap, 16) < 0)
                puts("here");

        vp = int2intmap_get(i2imap, 16);
        if (vp != NULL)
                printf("%d\n", *vp);

        int2intmap_free(&i2imap);
}

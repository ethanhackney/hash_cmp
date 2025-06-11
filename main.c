#include "include/pld_hash_map.h"
#include <stdio.h>

#define inthash(_n) ((size_t)(_n))
#define intcmp(_a, _b) (((_a) > (_b)) - ((_a) < (_b)))

PLD_HASH_MAP_DEFINE(int, int, int2intmap, inthash, intcmp)

int
main(void)
{
        struct int2intmap *i2imap = NULL;

        i2imap = int2intmap_new(0);
        if (i2imap == NULL)
                puts("here");

        int2intmap_free(&i2imap);
}

#include "include/pld_hash_map.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>

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

static int key[1 << 24] = {0};
static int val[1 << 24] = {0};

int
main(void)
{
        struct int2intmap *i2imap = NULL;
        clock_t start = 0;
        int nkey = sizeof(key) / sizeof(*key);
        int *vp = NULL;
        int i = 0;

        i2imap = int2intmap_new(0);
        assert(i2imap != NULL);

        srand((unsigned int)time(NULL));

        for (i = 0; i < nkey; i++) {
                key[i] = rand();
                val[i] = rand();
        }

        start = clock();

        for (i = 0; i < nkey; i++) {
                assert(int2intmap_set(&i2imap, key[i], val[i]) == 0);
                vp = int2intmap_get(i2imap, key[i]);
                assert(vp != NULL);
                assert(*vp == val[i]);
        }

        for (i = 0; i < nkey; i++) {
                vp = int2intmap_get(i2imap, key[i]);
                if (vp != NULL)
                        assert(int2intmap_unset(&i2imap, key[i]) == 0);
        }

        printf("%lf\n", ((double)clock() - (double)start) / CLOCKS_PER_SEC);

        int2intmap_free(&i2imap);
}

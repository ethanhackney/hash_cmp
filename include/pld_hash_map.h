#ifndef PLD_HASH_MAP_H
#define PLD_HASH_MAP_H

#include "util.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* initial capacity of pld_hash_map */
#ifndef PLD_HASH_MAP_INIT_CAP
#define PLD_HASH_MAP_INIT_CAP 32
#endif /* #ifndef PLD_HASH_MAP_INIT_CAP */

/* slot metadata */
enum {
        PLD_HASH_MAP_NEVER = 0xff, /* slot never occupied */
        PLD_HASH_MAP_WAS   = 0xfe, /* slot was occupied */
};

/**
 * Define a new hash table with linear displacement probing:
 *
 * Arguments:
 *  @_link: linkage of generated functions
 *  @_k:    key type
 *  @_v:    value type
 *  @_name: name of generated struct and prefix of all function names
 *  @_hash: hash function
 *  @_cmp:  key comparison function
 */
#define PLD_HASH_MAP_DEFINE(_link, _k, _v, _name, _hash, _cmp)  \
                                                                \
/* hash table with linear displacement probing */               \
struct _name {                                                  \
        uint8_t *p_meta; /* slot metadata */                    \
        size_t   p_cap;  /* capacity */                         \
        size_t   p_len;  /* entry count */                      \
        _k      *p_key;  /* keys */                             \
        _v      *p_val;  /* values */                           \
};                                                              \
                                                                \
/**                                                             \
 * Create a new _name{}:                                        \
 *                                                              \
 * Arguments:                                                   \
 *  @cap: initial capacity (or 0 for default)                   \
 *                                                              \
 * Returns:                                                     \
 *  @success: pointer to _name{}                                \
 *  @failure: NULL and errno set                                \
 */                                                             \
_link struct _name *                                            \
_name ## _new(size_t cap)                                       \
{                                                               \
        struct _name *pp = malloc(sizeof(*pp));                 \
        size_t msize = 0;                                       \
                                                                \
        if (pp == NULL)                                         \
                goto ret;                                       \
                                                                \
        if (cap == 0)                                           \
                cap = PLD_HASH_MAP_INIT_CAP;                    \
        else                                                    \
                cap = next_pow2(cap);                           \
                                                                \
        msize = sizeof(*pp->p_meta) * cap;                      \
        pp->p_meta = malloc(msize);                             \
        if (pp->p_meta == NULL)                                 \
                goto free_pp;                                   \
        memset(pp->p_meta, PLD_HASH_MAP_NEVER, msize);          \
                                                                \
        pp->p_key = malloc(sizeof(_k) * cap);                   \
        if (pp->p_key == NULL)                                  \
                goto free_meta;                                 \
                                                                \
        pp->p_val = malloc(sizeof(_v) * cap);                   \
        if (pp->p_val == NULL)                                  \
                goto free_key;                                  \
                                                                \
        goto ret;                                               \
                                                                \
free_key:                                                       \
        free(pp->p_key);                                        \
        pp->p_key = NULL;                                       \
                                                                \
free_meta:                                                      \
        free(pp->p_meta);                                       \
        pp->p_meta = NULL;                                      \
                                                                \
free_pp:                                                        \
        free(pp);                                               \
        pp = NULL;                                              \
                                                                \
ret:                                                            \
        return pp;                                              \
}                                                               \
                                                                \
/**                                                             \
 * Free a _name{}:                                              \
 *                                                              \
 * Arguments:                                                   \
 *  @ppp: pointer to pointer to _name{}                         \
 *                                                              \
 * Returns:                                                     \
 *  @success: *ppp set to NULL                                  \
 *  @failure: does not                                          \
 */                                                             \
_link void                                                      \
_name ## _free(struct _name **ppp)                              \
{                                                               \
        struct _name *pp = *ppp;                                \
                                                                \
        free(pp->p_val);                                        \
        free(pp->p_key);                                        \
        free(pp->p_meta);                                       \
        free(pp);                                               \
        *ppp = NULL;                                            \
}

#endif /* #ifndef PLD_HASH_MAP_H */

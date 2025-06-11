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
 *  @_k:    key type
 *  @_v:    value type
 *  @_name: name of generated struct and prefix of all function names
 *  @_hash: hash function
 *  @_cmp:  key comparison function
 */
#define PLD_HASH_MAP_DEFINE(_k, _v, _name, _hash, _cmp)                 \
                                                                        \
/* hash table with linear displacement probing */                       \
struct _name {                                                          \
        hash_map_size_t  p_cap;  /* capacity */                         \
        hash_map_size_t  p_len;  /* entry count */                      \
        hash_map_size_t  p_was;  /* number of slots marked as WAS */    \
        hash_map_size_t *p_hash; /* saved hashes */                     \
        uint8_t         *p_meta; /* slot metadata */                    \
        _k              *p_key;  /* keys */                             \
        _v              *p_val;  /* values */                           \
};                                                                      \
                                                                        \
/**                                                                     \
 * Create a new _name{}:                                                \
 *                                                                      \
 * Arguments:                                                           \
 *  @cap: initial capacity (or 0 for default)                           \
 *                                                                      \
 * Returns:                                                             \
 *  @success: pointer to _name{}                                        \
 *  @failure: NULL and errno set                                        \
 */                                                                     \
static inline struct _name *                                            \
_name ## _new(hash_map_size_t cap)                                      \
{                                                                       \
        struct _name *pp = malloc(sizeof(*pp));                         \
        size_t msize = 0;                                               \
                                                                        \
        if (pp == NULL)                                                 \
                goto ret;                                               \
                                                                        \
        if (cap == 0)                                                   \
                cap = PLD_HASH_MAP_INIT_CAP;                            \
        else                                                            \
                cap = next_pow2(cap);                                   \
                                                                        \
        msize = sizeof(*pp->p_meta) * cap;                              \
        pp->p_meta = malloc(msize);                                     \
        if (pp->p_meta == NULL)                                         \
                goto free_pp;                                           \
        memset(pp->p_meta, PLD_HASH_MAP_NEVER, msize);                  \
                                                                        \
        pp->p_hash = malloc(sizeof(*pp->p_hash) * cap);                 \
        if (pp->p_hash == NULL)                                         \
                goto free_meta;                                         \
                                                                        \
        pp->p_key = malloc(sizeof(_k) * cap);                           \
        if (pp->p_key == NULL)                                          \
                goto free_hash;                                         \
                                                                        \
        pp->p_val = malloc(sizeof(_v) * cap);                           \
        if (pp->p_val == NULL)                                          \
                goto free_key;                                          \
                                                                        \
        pp->p_cap = cap;                                                \
        goto ret;                                                       \
                                                                        \
free_key:                                                               \
        free(pp->p_key);                                                \
        pp->p_key = NULL;                                               \
                                                                        \
free_hash:                                                              \
        free(pp->p_hash);                                               \
        pp->p_hash = NULL;                                              \
                                                                        \
free_meta:                                                              \
        free(pp->p_meta);                                               \
        pp->p_meta = NULL;                                              \
                                                                        \
free_pp:                                                                \
        free(pp);                                                       \
        pp = NULL;                                                      \
                                                                        \
ret:                                                                    \
        return pp;                                                      \
}                                                                       \
                                                                        \
/**                                                                     \
 * Free a _name{}:                                                      \
 *                                                                      \
 * Arguments:                                                           \
 *  @ppp: pointer to pointer to _name{}                                 \
 *                                                                      \
 * Returns:                                                             \
 *  @success: *ppp set to NULL                                          \
 *  @failure: does not                                                  \
 */                                                                     \
static inline void                                                      \
_name ## _free(struct _name **ppp)                                      \
{                                                                       \
        struct _name *pp = *ppp;                                        \
                                                                        \
        free(pp->p_val);                                                \
        pp->p_val = NULL;                                               \
                                                                        \
        free(pp->p_key);                                                \
        pp->p_key = NULL;                                               \
                                                                        \
        free(pp->p_hash);                                               \
        pp->p_hash = NULL;                                              \
                                                                        \
        free(pp->p_meta);                                               \
        pp->p_meta = NULL;                                              \
                                                                        \
        free(pp);                                                       \
        pp = NULL;                                                      \
                                                                        \
        *ppp = NULL;                                                    \
}                                                                       \
                                                                        \
/**                                                                     \
 * Resize _name{}:                                                      \
 *                                                                      \
 * Arguments:                                                           \
 *  @ppp: pointer to pointer to _name{}                                 \
 *  @cap: new capacity                                                  \
 *                                                                      \
 * Returns:                                                             \
 *  @success: 0                                                         \
 *  @failure: -1 and errno set                                          \
 */                                                                     \
static inline int                                                       \
_name ## _resize(struct _name **ppp, hash_map_size_t cap)               \
{                                                                       \
        struct _name *newpp = _name ## _new(cap);                       \
        struct _name *pp = *ppp;                                        \
        hash_map_size_t moved = 0;                                      \
        hash_map_size_t j = 0;                                          \
        hash_map_size_t i = 0;                                          \
        uint8_t tmp_disp = 0;                                           \
        uint8_t disp = 0;                                               \
        _k tmp_k = (_k){0};                                             \
        _k k = (_k){0};                                                 \
        _v tmp_v = (_v){0};                                             \
        _v v = (_v){0};                                                 \
                                                                        \
        if (newpp == NULL)                                              \
                return -1;                                              \
                                                                        \
        newpp->p_len = pp->p_len;                                       \
        newpp->p_was = pp->p_was;                                       \
                                                                        \
        for (i = 0; moved < pp->p_len; i++) {                           \
                if (pp->p_meta[i] >= PLD_HASH_MAP_WAS)                  \
                        continue;                                       \
                                                                        \
                k = pp->p_key[i];                                       \
                v = pp->p_val[i];                                       \
                disp = 0;                                               \
                j = pp->p_hash[i] & (cap - 1);                          \
                while (newpp->p_meta[j] != PLD_HASH_MAP_NEVER) {        \
                        if (newpp->p_meta[j] < disp) {                  \
                                tmp_k = newpp->p_key[j];                \
                                tmp_v = newpp->p_val[j];                \
                                tmp_disp = newpp->p_meta[j];            \
                                                                        \
                                newpp->p_key[j] = k;                    \
                                newpp->p_val[j] = v;                    \
                                newpp->p_meta[j] = disp;                \
                                                                        \
                                k = tmp_k;                              \
                                v = tmp_v;                              \
                                disp = tmp_disp;                        \
                        }                                               \
                        j = (j + 1) & (cap - 1);                        \
                        disp++;                                         \
                }                                                       \
                                                                        \
                newpp->p_hash[j] = pp->p_hash[i];                       \
                newpp->p_key[j] = k;                                    \
                newpp->p_val[j] = v;                                    \
                newpp->p_meta[j] = disp;                                \
                moved++;                                                \
        }                                                               \
                                                                        \
        _name ## _free(ppp);                                            \
        *ppp = newpp;                                                   \
        return 0;                                                       \
}

#endif /* #ifndef PLD_HASH_MAP_H */

#ifndef PLD_HASH_MAP_H
#define PLD_HASH_MAP_H

#include "util.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* initial capacity of pld_hash_map */
#ifndef PLD_HASH_MAP_INIT_CAP
#define PLD_HASH_MAP_INIT_CAP 32
#endif /* #ifndef PLD_HASH_MAP_INIT_CAP */

/* misc. constants */
enum {
        PLD_HASH_MAP_LOAD_FACTOR = 12, /* load factor */
};

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
        pp->p_len = 0;                                                  \
        pp->p_was = 0;                                                  \
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
        hash_map_size_t mask = 0;                                       \
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
        mask = newpp->p_cap - 1;                                        \
                                                                        \
        for (i = 0; moved < pp->p_len; i++) {                           \
                if (pp->p_meta[i] >= PLD_HASH_MAP_WAS)                  \
                        continue;                                       \
                                                                        \
                k = pp->p_key[i];                                       \
                v = pp->p_val[i];                                       \
                disp = 0;                                               \
                j = pp->p_hash[i] & mask;                               \
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
                        j = (j + 1) & mask;                             \
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
}                                                                       \
                                                                        \
/**                                                                     \
 * Test if rehash need to grow:                                         \
 *                                                                      \
 * Arguments:                                                           \
 *  @pp: pointer to _name{}                                             \
 *                                                                      \
 * Returns:                                                             \
 *  @true:  if needed                                                   \
 *  @false: if not                                                      \
 */                                                                     \
static inline bool                                                      \
_name ## _need_to_grow(const struct _name *pp)                          \
{                                                                       \
        hash_map_size_t len = pp->p_len;                                \
        hash_map_size_t cap = pp->p_cap;                                \
        hash_map_size_t was = pp->p_was;                                \
                                                                        \
        return ((len + was) << 4) > (cap * PLD_HASH_MAP_LOAD_FACTOR);   \
}                                                                       \
                                                                        \
/**                                                                     \
 * Set map[k] to v _name{}:                                             \
 *                                                                      \
 * Arguments:                                                           \
 *  @ppp: pointer to pointer to _name{}                                 \
 *  @k:   key                                                           \
 *  @v:   value                                                         \
 *                                                                      \
 * Returns:                                                             \
 *  @success: 0                                                         \
 *  @failure: -1 and errno set                                          \
 */                                                                     \
static inline int                                                       \
_name ## _set(struct _name **ppp, _k k, _v v)                           \
{                                                                       \
        struct _name *pp = *ppp;                                        \
        hash_map_size_t hash = _hash(k);                                \
        hash_map_size_t mask = pp->p_cap - 1;                           \
        hash_map_size_t i = hash & mask;                                \
        uint8_t tmp_disp = 0;                                           \
        uint8_t disp = 0;                                               \
        _k tmp_k = (_k){0};                                             \
        _v tmp_v = (_v){0};                                             \
                                                                        \
        if (unlikely(_name ## _need_to_grow(pp))) {                     \
                if (_name ## _resize(ppp, pp->p_cap << 1) < 0)          \
                        return -1;                                      \
                pp = *ppp;                                              \
                mask = pp->p_cap - 1;                                   \
                i = hash & mask;                                        \
        }                                                               \
                                                                        \
        for (;;) {                                                      \
                tmp_disp = pp->p_meta[i];                               \
                                                                        \
                if (tmp_disp >= PLD_HASH_MAP_WAS) {                     \
                        pp->p_key[i] = k;                               \
                        pp->p_val[i] = v;                               \
                        pp->p_meta[i] = disp;                           \
                                                                        \
                        if (tmp_disp == PLD_HASH_MAP_NEVER)             \
                                pp->p_len++;                            \
                        else                                            \
                                pp->p_was--;                            \
                                                                        \
                        return 0;                                       \
                }                                                       \
                                                                        \
                if (_cmp(pp->p_key[i], k) == 0) {                       \
                        pp->p_val[i] = v;                               \
                        return 0;                                       \
                }                                                       \
                                                                        \
                if (tmp_disp < disp) {                                  \
                        tmp_k = pp->p_key[i];                           \
                        tmp_v = pp->p_val[i];                           \
                                                                        \
                        pp->p_key[i] = k;                               \
                        pp->p_val[i] = v;                               \
                        pp->p_meta[i] = disp;                           \
                                                                        \
                        k = tmp_k;                                      \
                        v = tmp_v;                                      \
                        disp = tmp_disp;                                \
                }                                                       \
                                                                        \
                i = (i + 1) & mask;                                     \
                disp++;                                                 \
                if (unlikely(disp >= PLD_HASH_MAP_WAS))                 \
                        return -1;                                      \
        }                                                               \
}                                                                       \
                                                                        \
/**                                                                     \
 * Get map[k] from _name{}:                                             \
 *                                                                      \
 * Arguments:                                                           \
 *  @pp: pointer to pointer to _name{}                                  \
 *  @k:  key                                                            \
 *                                                                      \
 * Returns:                                                             \
 *  @success: pointer to _v{}                                           \
 *  @failure: NULL                                                      \
 */                                                                     \
static inline _v *                                                      \
_name ## _get(struct _name *pp, _k k)                                   \
{                                                                       \
        hash_map_size_t hash = _hash(k);                                \
        hash_map_size_t mask = pp->p_cap - 1;                           \
        hash_map_size_t i = hash & mask;                                \
        uint8_t cur_disp = 0;                                           \
        uint8_t disp = 0;                                               \
                                                                        \
        for (;;) {                                                      \
                cur_disp = pp->p_meta[i];                               \
                                                                        \
                if (cur_disp == PLD_HASH_MAP_NEVER)                     \
                        return NULL;                                    \
                if (cur_disp < disp)                                    \
                        return NULL;                                    \
                                                                        \
                if (cur_disp != PLD_HASH_MAP_WAS) {                     \
                        if (_cmp(pp->p_key[i], k) == 0)                 \
                                return &pp->p_val[i];                   \
                }                                                       \
                                                                        \
                i = (i + 1) & mask;                                     \
                disp++;                                                 \
                if (unlikely(disp >= PLD_HASH_MAP_WAS))                 \
                        return NULL;                                    \
        }                                                               \
}                                                                       \
                                                                        \
/**                                                                     \
 * Test if need to shrink:                                              \
 *                                                                      \
 * Arguments:                                                           \
 *  @pp: pointer to _name{}                                             \
 *                                                                      \
 * Returns:                                                             \
 *  @true:  if needed                                                   \
 *  @false: if not                                                      \
 */                                                                     \
static inline bool                                                      \
_name ## _need_to_shrink(const struct _name *pp)                        \
{                                                                       \
        hash_map_size_t len = pp->p_len;                                \
        hash_map_size_t cap = pp->p_cap;                                \
                                                                        \
        return len <= (cap >> 1) && (cap > PLD_HASH_MAP_INIT_CAP);      \
}                                                                       \
                                                                        \
/**                                                                     \
 * Unset map[k] _name{}:                                                \
 *                                                                      \
 * Arguments:                                                           \
 *  @ppp: pointer to pointer to _name{}                                 \
 *  @k:   key                                                           \
 *                                                                      \
 * Returns:                                                             \
 *  @success: 0                                                         \
 *  @failure: -1 and errno set                                          \
 */                                                                     \
static inline int                                                       \
_name ## _unset(struct _name **ppp, _k k)                               \
{                                                                       \
        struct _name *pp = *ppp;                                        \
        hash_map_size_t hash = _hash(k);                                \
        hash_map_size_t mask = pp->p_cap - 1;                           \
        hash_map_size_t i = hash & mask;                                \
        uint8_t cur_disp = 0;                                           \
        uint8_t disp = 0;                                               \
                                                                        \
        if (unlikely(_name ## _need_to_shrink(pp))) {                   \
                if (_name ## _resize(ppp, pp->p_cap >> 1) < 0)          \
                        return -1;                                      \
                pp = *ppp;                                              \
                mask = pp->p_cap - 1;                                   \
                i = hash & mask;                                        \
        }                                                               \
                                                                        \
        for (;;) {                                                      \
                cur_disp = pp->p_meta[i];                               \
                                                                        \
                if (cur_disp == PLD_HASH_MAP_NEVER)                     \
                        return 0;                                       \
                if (cur_disp < disp)                                    \
                        return 0;                                       \
                                                                        \
                if (_cmp(pp->p_key[i], k) == 0) {                       \
                        pp->p_meta[i] = PLD_HASH_MAP_WAS;               \
                        pp->p_len--;                                    \
                        pp->p_was++;                                    \
                        return 0;                                       \
                }                                                       \
                                                                        \
                i = (i + 1) & mask;                                     \
                disp++;                                                 \
                if (unlikely(disp >= PLD_HASH_MAP_WAS))                 \
                        return -1;                                      \
        }                                                               \
}

#endif /* #ifndef PLD_HASH_MAP_H */

/*
 * qsort-common.h --
 * Copyright (C) 2015 Daniel Santos <daniel.santos@pobox.com>
 * This file is part of gboing.

 * gboing is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * gboing is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser Public License for more details.

 * You should have received a copy of the GNU Lesser Public License
 * along with gboing.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _QSORT_COMMON_H
#define _QSORT_COMMON_H

#include "gboing/qsort-template.h"
#include "test-common.h"

#ifndef ELEM_SIZE
# define ELEM_SIZE 4096
#endif

#ifndef ALIGN_SIZE
# define ALIGN_SIZE 4
#endif

#ifndef KEY_SIGN
# define KEY_SIGN uint
#endif

#ifndef LESS_FN
# define LESS_FN compar_r
#endif

#ifndef OUTLINE_COPY
# define OUTLINE_COPY 0
#endif

#ifndef OUTLINE_SWAP
# define OUTLINE_SWAP 0
#endif

#ifndef SUPPLY_BUFFER
# define SUPPLY_BUFFER 0
#endif

#ifndef MAX_SIZE_BITS
# define MAX_SIZE_BITS 0
#endif

#ifndef MAX_THRESH
# define MAX_THRESH 0
#endif

static const unsigned KEY_BITS =
    ELEM_SIZE >= 8 && ((ELEM_SIZE) % gboing_alignof(uint64_t)) == 0
    ? 64
    : ELEM_SIZE >= 4 && ((ELEM_SIZE) % gboing_alignof(uint32_t)) == 0
    ? 32
    : ELEM_SIZE >= 2 && ((ELEM_SIZE) % gboing_alignof(uint16_t)) == 0
    ? 16
    : 8;

/* packing isn't normally something one would do, but we need to force the
 * specified object size, so we use pack(1) to get that */
#pragma pack(push,1)
struct size_type {
    char c[ELEM_SIZE];
} gboing_aligned(ALIGN_SIZE);
#pragma pack(pop)

#define key_type(bits) GBOING_EXPAND_ARGS(GBOING_CAT3, KEY_SIGN, bits, _t)

#define gboing_finite_compare(a, b, type, align)  ({        \
    const type *_a = gboing_assume_aligned((a), (align));   \
    const type *_b = gboing_assume_aligned((b), (align));   \
    *_a > *_b ? 1 : (*_a < *_b ? -1 : 0);                   \
})


__always_inline static int my_compar(const void *a, const void *b) {
    switch (KEY_BITS) {
        case 8 : return gboing_finite_compare(a, b, key_type(8), ALIGN_SIZE);
        case 16: return gboing_finite_compare(a, b, key_type(16), ALIGN_SIZE);
        case 32: return gboing_finite_compare(a, b, key_type(32), ALIGN_SIZE);
        case 64: return gboing_finite_compare(a, b, key_type(64), ALIGN_SIZE);
        default:
            gboing_assert(0);
            return 0;
    }
}

static __always_inline int my_less(const void *a, const void *b) {
    switch (KEY_BITS) {
        case 8: {
            const key_type(8) *_a = __builtin_assume_aligned(a, ALIGN_SIZE);
            const key_type(8) *_b = __builtin_assume_aligned(b, ALIGN_SIZE);
            return *_a < *_b;
        }
        case 16: {
            const key_type(16) *_a = __builtin_assume_aligned(a, ALIGN_SIZE);
            const key_type(16) *_b = __builtin_assume_aligned(b, ALIGN_SIZE);
            return *_a < *_b;
        }
        case 32: {
            const key_type(32) *_a = __builtin_assume_aligned(a, ALIGN_SIZE);
            const key_type(32) *_b = __builtin_assume_aligned(b, ALIGN_SIZE);
            return *_a < *_b;
        }
        case 64: {
            const key_type(64) *_a = __builtin_assume_aligned(a, ALIGN_SIZE);
            const key_type(64) *_b = __builtin_assume_aligned(b, ALIGN_SIZE);
            return *_a < *_b;
        }
        default:
            gboing_assert(0);
            return 0;
    }
}

static __always_inline int my_compar_r(const void *a, const void *b, void *context) {
    switch (KEY_BITS) {
        case 8 : return gboing_finite_compare(a, b, key_type(8), ALIGN_SIZE);
        case 16: return gboing_finite_compare(a, b, key_type(16), ALIGN_SIZE);
        case 32: return gboing_finite_compare(a, b, key_type(32), ALIGN_SIZE);
        case 64: return gboing_finite_compare(a, b, key_type(64), ALIGN_SIZE);
        default:
            gboing_assert(0);
            return 0;
    }
}

static __always_inline int my_less_r(const void *a, const void *b, void *context) {
    switch (KEY_BITS) {
        case 8: {
            const key_type(8) *_a = __builtin_assume_aligned(a, ALIGN_SIZE);
            const key_type(8) *_b = __builtin_assume_aligned(b, ALIGN_SIZE);
            return *_a < *_b;
        }
        case 16: {
            const key_type(16) *_a = __builtin_assume_aligned(a, ALIGN_SIZE);
            const key_type(16) *_b = __builtin_assume_aligned(b, ALIGN_SIZE);
            return *_a < *_b;
        }
        case 32: {
            const key_type(32) *_a = __builtin_assume_aligned(a, ALIGN_SIZE);
            const key_type(32) *_b = __builtin_assume_aligned(b, ALIGN_SIZE);
            return *_a < *_b;
        }
        case 64: {
            const key_type(64) *_a = __builtin_assume_aligned(a, ALIGN_SIZE);
            const key_type(64) *_b = __builtin_assume_aligned(b, ALIGN_SIZE);
            return *_a < *_b;
        }
        default:
            gboing_assert(0);
            return 0;
    }
}

static gboing_unused void randomize(void *p, size_t n, size_t size, unsigned int seed) {
    unsigned long *arr = p;
    const size_t LONG_BITS = sizeof(unsigned long) * 8;
    const size_t RAND_BITS = LONG_BITS - __builtin_clzl((unsigned long)RAND_MAX);
    const size_t bytes = n * size;
    const size_t count = bytes / sizeof(unsigned long);
    size_t i;

    //assert(!(bytes % sizeof(*arr)));
    //assert(size == sizeof(*arr));

    srandom(seed);

    for (i = 0; i < count; ++i) {
        size_t bits;

        arr[i] = (unsigned long)random();

        for (bits = RAND_BITS; bits < LONG_BITS; bits += RAND_BITS) {
            arr[i] <<= RAND_BITS;
            arr[i] ^= (unsigned long)random();
        }
//      arr[i] >>= 56;
    }

    /* if not aligned to size of long get the last few bytes */
    for (i = 0; i < bytes % sizeof(*arr); ++i) {
        ((char *)p) [count * sizeof(*arr) + i] = random();
    }
}

void gboing_noinline my_elem_copy(void *dest, const void *src) {
    memcpy(gboing_assume_aligned(dest, ALIGN_SIZE),
           gboing_assume_aligned(src, ALIGN_SIZE),
           ELEM_SIZE);
}

void gboing_noinline my_elem_swap(void *tmp, void *a, void *b) {
    struct size_type *_tmp = gboing_assume_aligned(tmp, ALIGN_SIZE);
    struct size_type *_a = gboing_assume_aligned(a, ALIGN_SIZE);
    struct size_type *_b = gboing_assume_aligned(b, ALIGN_SIZE);

    *_tmp = *_a;
    *_a   = *_b;
    *_b   = *_tmp;
}

static const struct qsort_def my_def = {
    .size          = ELEM_SIZE,
    .align         = ALIGN_SIZE,
    .LESS_FN       = GBOING_CAT(my_, LESS_FN),
#if MAX_SIZE_BITS
    .max_size_bits = MAX_SIZE_BITS,
#endif
#if OUTLINE_COPY
    .elem_copy     = my_elem_copy,
#endif
#if OUTLINE_SWAP
    .elem_swap     = my_elem_swap,
#endif
#if MAX_THRESH
    .max_thresh    = MAX_THRESH,
#endif
    //.buf_size  = 0x10000,
    //.buf_align = 32
};

typedef int (*compar_t)(const void *, const void *, void *arg);
typedef void (*sort_func_t)(void *p, size_t n, size_t size, compar_t compar, void *arg);


#endif /* _QSORT_COMMON_H */

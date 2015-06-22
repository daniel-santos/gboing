/**
 * @file align.h
 * @breif functions & macros for working with aligned memory
 */

/* Copyright (C) 2014-1015 Daniel Santos <daniel.santos@pobox.com>
 * This file is part of gboing.
 *
 * gboing is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * gboing is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser Public License for more details.
 *
 * You should have received a copy of the GNU Lesser Public License
 * along with gboing.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <gboing/compiler.h>
#include <gboing/assert.h>

#ifdef __GLIBC__
# define GBOING_MIN_ALIGN (sizeof(void *) * 2)
#else
# define GBOING_MIN_ALIGN 1
#endif

static gboing_always_inline gboing_flatten void *
gboing_aligned_memcpy(void *dest, const void *src, size_t n, size_t align) {
    return memcpy(gboing_assume_aligned(dest, align),
                  gboing_assume_aligned(src, align),
                  n);
}

static inline void *
gboing_align_pointer(void *ptr, size_t align) {
    const size_t a = align - 1;
    gboing_assert_early(gboing_is_pow2(align));
    return (void *) (((uintptr_t)ptr + a) & ~a);
}


/* Select an aligned_alloc strategy based upon what is or isn't available */
#if defined(_ISOC11_SOURCE)
/* C11 is easiest */

# define gboing_aligned_alloc(align, n) aligned_alloc(align, n)
# define gboing_aligned_free(ptr)       free(ptr)
#elif (defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200112L) \
        || (defined(_XOPEN_SOURCE) && _XOPEN_SOURCE >= 600)
/* Next is POSIX 2001 (Issue 6) */

static gboing_unused void *gboing_aligned_alloc(size_t align, size_t n) {
    void *ret;

    align = gboing_max(align, sizeof(void*));

    /* errors resulting from align not being a power of two are treated as
     * ENOMEM here as they should be caught elsewhere */
    return posix_memalign(&ret, align, n) ? NULL : ret;
}

# define gboing_aligned_free(ptr) free(ptr)
#else /* not C11 or POSIX 2001 */
/* Otherwise, we'll use the home-grown version that wastes sizeof(void*) +
 * align - 1 bytes with each allocation. Here, we store the original pointer
 * at the start of the buffer for later use by free. */

static gboing_unused void *gboing_aligned_alloc(size_t align, size_t n) {
    void *ptr = malloc(n + sizeof(void*) + align - 1);
    void **ret;

    if (!ptr)
        return NULL;

    ret = gboing_align_pointer(ptr + sizeof(void *), align);
    ret[-1] = ptr;
    return ret;
}

static gboing_unused void gboing_aligned_free(void *ptr) {
    free(((void**)ptr)[-1]);
}

#endif /* gboing_aligned_alloc and gboing_aligned_free */


#ifndef gboing_aligned
# define gboing_aligned(n)
#endif
#ifndef gboing_alignof
# define gboing_alignof(expr)               (1)
#endif
#ifndef gboing_alloc_size
# define gboing_alloc_size(...)
#endif
#ifndef gboing_assume_aligned
# define gboing_assume_aligned(p, ...)      (p)
#endif
#ifndef gboing_aligned_alloca
# define gboing_aligned_alloca(align, n) \
    gboing_aligned_alloca_fallback((align), (n))
#endif

/* Fallback aligned alloca macro for when __builtin_alloca_with_align, is
 * bugged or not available. This is less efficient because it wastes space to
 * assure alignment.
 */
#ifdef GBOING_DEBUG_ALLIGNED_ALLOCA
# define gboing_aligned_alloca_fallback(align, n) ({                    \
        void *ret;                                                      \
        fprintf(stderr, "gboing_aligned_alloca with n=%lu, "            \
                        "align=%lu\n", (n), (align));                   \
        ret = gboing_align_pointer(alloca((n) + (align) - 1), (align)); \
        assert(ret);                                                    \
        ret;                                                            \
    })
# else
# define gboing_aligned_alloca_fallback(align, n) \
    gboing_align_pointer(alloca((n) + (align) - 1), (align))
#endif

/* Copyright (C) 1991,1992,1996,1997,1999,2004 Free Software Foundation, Inc.
 * Copyright (C) 2014-1015 Daniel Santos <daniel.santos@pobox.com>
 *
 * The GNU C Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * The GNU C Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the GNU C Library; if not, see
 * <http://www.gnu.org/licenses/>.
 */

/* If you consider tuning this algorithm, you should consult first:
   Engineering a sort function; Jon Bentley and M. Douglas McIlroy;
   Software - Practice and Experience; Vol. 23 (11), 1249-1265, 1993.  */

/**
 * @file
 * @breif A qsort C metafunction
 * qsort-template.h is based off of stdlib/qsort.c from the the GNU C Library
 * version 2.17 originally written by Douglas C. Schmidt (schmidt@ics.uci.edu).
 * It has been modified to implement the qsort algorithm as a C pseudo-template
 * function (C metafunction).
 *
 * Note that glibc doesn't normally use stdlib/qsort.c when qsort or qsort_r is
 * called, a more efficient msort algo is used when possible. However, msort
 * cannot be properly converted into a metafunction since it uses recursion --
 * a barrier across which gcc cannot fold constants (at this time).
 */

#ifndef _QSORT_TEMPLATE_H_
#define _QSORT_TEMPLATE_H_

#define _GNU_SOURCE
#include <stddef.h>
#include <alloca.h>
#include <limits.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include <gboing/align.h>
#include <gboing/assert.h>

#if __STDC_VERSION__ >= 201112L
# include <stdalign.h>
# define _QSORT_ALIGN_MAX _Alignof(max_align_t)
#else
# define _QSORT_ALIGN_MAX 128
#endif

/* Do a little CPU sniffing to try determine most efficient algo. ARM uses LSR
 * extension to cover any index that's a power of two, but intel only has 1, 2,
 * 4, and 8.
 * FIXME: Only ARM & x86 are considered here. Tweaks for other archs would
 * be helpful.
 */
#if defined(__i386) || defined (__i386__)
# define _QSORT_ARCH_MAX_INDEX_MULT 8
#endif

#ifndef _QSORT_ARCH_MAX_INDEX_MULT
# define _QSORT_ARCH_MAX_INDEX_MULT __SIZE_MAX__
#endif

/* Threshold of element size before switching to an indirect sort. */
#ifndef _QSORT_IND_THRESH
# define _QSORT_IND_THRESH 64
#endif


/**
 * @struct qsort_def
 * @brief pseudo-template definition and params for qsort_template
 * @var qsort_def::size
 * Element size
 *
 * @var qsort_def::align
 * Minimum alignment of elements
 *
 * @var qsort_def::less
 * @var qsort_def::compar
 * @var qsort_def::less_r
 * @var qsort_def::compar_r
 *
 * @var qsort_def::elem_copy
 * (Optional) Pointer to a function to copy an element. This can be useful to
 * override the default implementation in cases where the element size is very
 * large. Overriding with either a noinline function or an inline function that
 * calls a noinline function can reduce the size of the instantiated
 * qsort_template function dramatically by forcing the copy to be outlined.
 * Note that there are cases where even a large element is copied with a small
 * amount of code (e.g., x86's rep mov, etc.). Unfortunately, gcc currently
 * offers no mechanism to auto-generate a copy function and allow the compiler
 * to decide if it should be inlined or not. :(
 *
 * For optimal performance in an outlined copy function, the alignment of
 * element pointers passed should be explicitly dictated to the compiler via
 * __builtin_assume_aligned() or some other means.
 *
 * @var qsort_def::elem_swap
 * (Optional) Pointer to a function to swap elements. Similar to elem_copy.
 * This function may use qsort_def::elem_buf if it chooses.
 *
 * @var qsort_def::elem_buf
 * Pointer to an element buffer (should be at least qsort_def::size bytes and
 * aligned to qsort_def::size).
 *
 * @var qsort_def::max_size_bits
 * Maximum number of bits needed to store count of elements. e.g., if
 * max_size_bits is 16, then the qsort will only allocate enough stack and/or
 * heap space to sort arrays with 2^16 - 1 (65535) elements in them. If unset,
 * will default to size(size_t) * 8 * 3 / 4 (essentially, 24 bits on a 32 bit
 * machine and 48 bits on a 64 bit machine). Set this to a lower value to reduce
 * memory usage at the cost of a restricted array size.
 *
 * @var qsort_def::max_stack
 * (currently unused) The maxiumum number of bytes to put on the stack. If
 * needed space bypasses this value, memory is allocated on the heap.
 *
 * @var qsort_def::max_thresh
 *
 * @var qsort_def::index
 * (internal) Pointer to an index buffer when indirect sorting is used.
 *
 * NOTES: alloca cannot be inlined via indirection (see comments):
 * https://github.com/gcc-mirror/gcc/blob/master/gcc/calls.c#L581
 */
struct qsort_def {
    size_t size;
    size_t align;
    int (*less)(const void *a, const void *b);
    int (*compar)(const void *a, const void *b);
    int (*less_r)(const void *a, const void *b, void *context);
    int (*compar_r)(const void *a, const void *b, void *context);
    void (*elem_copy)(void *dest, const void *src);
    void (*elem_swap)(void *elem_buf, void *a, void *b);
    void *elem_buf;
    size_t max_size_bits;
    size_t max_stack;
    size_t max_thresh;
    void *(*aligned_alloc)(size_t alignment, size_t size);
    void (*free)(void *buffer);

    void **index;
};

#if GCC_VERSION < 40700

/* fallback qsort_template function */
static int
qsort_template(const struct qsort_def *def, void *const pbase,
               size_t n, void *arg) {
    assert(!!def->compar || !!def->compar_r);

    if (!!def->compar_r)
        qsort_r(pbase, n, def->size, def->compar_r, arg);
    else
        qsort(pbase, n, def->size, def->compar);

    return 0;
}

#else /* GCC_VERSION >= 40700 */

/**
 * @breif Auto-generated element copy function.
 *
 *
 * @param def
 * @param dest
 * @param src
 */
//#define _qsort_copy(a, b, c)

static gboing_always_inline gboing_flatten void
_qsort_copy(const struct qsort_def *def, void *dest, const void *src) {
    gboing_assert_const(def->align);
    gboing_assert_const(!def->elem_copy);
    gboing_assert_const(def->size);

    if (!!def->elem_copy && !def->index)
        def->elem_copy (dest, src);
    else {

        /* A switch statement is used here to bring the value align from a
         * compile-time constant back into a literal constant usable by
         * __builtin_assume_aligned() and macro pasting. Through dead code
         * removal, the optimizer will remove the all code (the switch
         * statement, cases and bodies of non-matching cases) required for this
         * mutation. */
        switch (def->align) {
            case 1:   gboing_aligned_memcpy(dest, src, def->size, 1);
                      break;
            case 2:   gboing_aligned_memcpy(dest, src, def->size, 2);
                      break;
            case 4:   gboing_aligned_memcpy(dest, src, def->size, 4);
                      break;
            case 8:   gboing_aligned_memcpy(dest, src, def->size, 8);
                      break;
            case 16:  gboing_aligned_memcpy(dest, src, def->size, 16);
                      break;
            case 32:  gboing_aligned_memcpy(dest, src, def->size, 32);
                      break;
            case 64:  gboing_aligned_memcpy(dest, src, def->size, 64);
                      break;
            case 128: gboing_aligned_memcpy(dest, src, def->size, 128);
                      break;
            case 256: gboing_aligned_memcpy(dest, src, def->size, 256);
                      break;
            case 512: gboing_aligned_memcpy(dest, src, def->size, 512);
                      break;
            default:  gboing_assert(0);
        }
    }
}

static gboing_always_inline void
_qsort_swap(const struct qsort_def *def, void *a, void *b) {
    if (!!def->elem_swap && !def->index)
        def->elem_swap(def->elem_buf, a, b);
    else {
        _qsort_copy(def, def->elem_buf, a);
        _qsort_copy(def, a, b);
        _qsort_copy(def, b, def->elem_buf);
    }
}

/**
 * @brief Move data element at right to left, shifting all elements in between
 *        to the right.
 *
 * @param def         the template parameters
 * @param left        leftmost element
 * @param right       rightmost element
 */
static gboing_always_inline gboing_flatten void
_qsort_ror(const struct qsort_def *def, void *left, void *right) {
    const ssize_t size = (ssize_t)def->size;
    char *r = right;
    char *l = left;
    char *left_minus_one = l - size;
    const ssize_t dist = (r - l) / size; /* left to right offset */
    ssize_t i;

    /* validate pointer alignment */
    gboing_assert_early(!((uintptr_t)l & (def->align - 1)));
    gboing_assert_early(!((uintptr_t)r & (def->align - 1)));

    /* validate distance between pointers is positive */
    assert(dist != 0);
    assert(dist > 0);

    _qsort_copy(def, def->elem_buf, r);

    /* x86 rep movs-friendly loop */
    for (i = dist; i; --i)
        _qsort_copy(def, &l[i * size], &left_minus_one[i * size]);

    _qsort_copy(def, left, def->elem_buf);
}

/**
 * @breif instantiates a less function based upon supplied function pointer.
 *
 * @param def
 * @param indirect
 * @param a
 * @param b
 * @param arg
 */
static gboing_always_inline gboing_flatten int
_qsort_less(const struct qsort_def *def, void *a, void *b, void *arg) {

    if (!!def->index) {
        a = *((void**)a);
        b = *((void**)b);
    }

    /* determine which compar/less fn to call and adapt it */
    if (!!def->less)
        return def->less(a, b);
    else if (!!def->compar)
        return def->compar(a, b) < 0;
    else if (!!def->less_r)
        return def->less_r(a, b, arg);
    else if (!!def->compar_r)
        return def->compar_r(a, b, arg) < 0;
    else {
        gboing_assert_early(0);
        return 0;
    }
}

/* Discontinue qsort algorithm when partition gets below this size.
   This particular magic number was chosen to work best on a Sun 4/260. */
#define DEFAULT_MAX_THRESH 4

/* Stack node declarations used to store unfulfilled partition obligations. */
typedef struct {
    char *lo;
    char *hi;
} stack_node;

/* The _qsort_push, _qsort_pop  4 #defines implement a very fast in-line stack abstraction. */
/* The stack needs log (total_elements) entries (we could even subtract
   log(def.max_thresh)).
   */

static gboing_always_inline void
_qsort_push(stack_node **top, char *low, char *high) {
    (*top)->lo = low;
    (*top)->hi = high;
    ++(*top);
}

static gboing_always_inline void
_qsort_pop(stack_node **top, char **low, char **high) {
  --(*top);
  *low = (*top)->lo;
  *high = (*top)->hi;
}

/* Order size using qsort.  This implementation incorporates
   four optimizations discussed in Sedgewick:

   1. Non-recursive, using an explicit stack of pointer that store the
      next array partition to sort.  To save time, this maximum amount
      of space required to store an array of SIZE_MAX is allocated on the
      stack.  Assuming a 32-bit integer for size_t, this needs
      only 32 * sizeof(stack_node) == 256 bytes. For 64 bits, 768 bytes
      or 1024 bytes if uncapped.

   2. Chose the pivot element using a median-of-three decision tree.
      This reduces the probability of selecting a bad pivot value and
      eliminates certain extraneous comparisons.

   3. Only qsorts TOTAL_ELEMS / def.max_thresh partitions, leaving
      insertion sort to order the def.max_thresh items within each partition.
      This is a big win, since insertion sort is faster for small, mostly
      sorted array segments.

   4. The larger of the two sub-partitions is always pushed onto the
      stack first, with the algorithm then concentrating on the
      smaller partition.  This *guarantees* no more than log (n)
      stack size is needed (actually O(1) in this case)!  */

/**
 * @breif
 *
 * @param def
 *
 * @param buffer
 * (Optional) Temporary memory to use instead of allocating memory on the stack
 * and/or heap. If supplied, it should be aligned to the greater of
 * qsort_def::align or the __alignof__(void *) of a failed assertion / sigbus
 * may result. If not used, a compile-time constant NULL value should be passed
 * to reduce generated code.
 *
 * @param buf_size
 * Size of buffer (if non-null), zero otherwise. Using a compile-time constant
 * value will result in a small reduction in generated code size.
 *
 * @param pbase
 * Element array.
 *
 * @param n
 * Number of elements.
 *
 * @param arg
 * Contextual argument to pass to qsort_def::less_r() or qsort_def::compar_r()
 * function.
 */

static gboing_always_inline gboing_flatten int
qsort_template(const struct qsort_def *def, void *buffer, size_t buf_size,
               void *const pbase, size_t n, void *arg) {
    /* this copy of def will be mutated to manage indirect sorting if needed */
    struct qsort_def d = *def;
    char *base_ptr = (char *) pbase;
    size_t max_thresh;                      /* ct const */

    /* Use indirect sorting if size is large */
    const int indirect              = d.size > _QSORT_IND_THRESH; /* ct const */
    size_t tmp_needed               = 0;  /* can be either ct or rt value */
    size_t tmp_align                = 0;  /* ct const */
    size_t buf_used                 = 0;  /* ct const */
    stack_node *qstack              = 0;  /* rt value */
    const size_t QSTACK_ALIGN       = gboing_alignof(stack_node);
    const size_t PTR_ALIGN          = gboing_alignof(void *);
    size_t pad_size;                      /* ct const */
    size_t stack_used               = 0;  /* ct const -- note that we omit alignment padding */
    size_t qstack_size;                   /* ct const */
    size_t qstack_tmp_offset        = 0;  /* ct const */
    size_t index_tmp_offset         = 0;  /* ct const */
    void *tmp_buffer                = NULL; /* rt value */
    int elem_buf_is_set             = 0;  /*ct const -- a work-around for a bug */


    if (n == 0)
        /* Avoid lossage with unsigned arithmetic below.  */
        return 0;

    /* Restrict to reasonable value */
    if (d.align > _QSORT_ALIGN_MAX)
        d.align = _QSORT_ALIGN_MAX;

    if (d.max_size_bits) {
        d.max_size_bits = gboing_min(d.max_size_bits, sizeof(size_t) * 8);
    } else
        d.max_size_bits = sizeof(size_t) * 8 * 3 / 4;

    assert(n <= ((size_t)1 << d.max_size_bits) - 1);

    if (!d.max_thresh)
        d.max_thresh = DEFAULT_MAX_THRESH;

    if (!d.max_stack)
        d.max_stack = 1024;

//    if (d.align < GBOING_MIN_ALIGN)
//        d.align = GBOING_MIN_ALIGN;

#ifdef GNU_LIBC_SOME_SUCH
    if (d.max_stack > __MAX_ALLOCA_CUTOFF)
        d.max_stack = __MAX_ALLOCA_CUTOFF;
#endif

    d.index = NULL; /* ignore if it was populated by caller */

    /* validate required fields are constants */
    gboing_assert_const(!d.less + !d.compar + !d.less_r + !d.compar_r);
    gboing_assert_const(d.align);
    gboing_assert_const(d.size);
    gboing_assert_msg(!!d.less || !!d.compar || !!d.less_r || !!d.compar_r,
                      "a less or compar function is required");

    gboing_assert(gboing_is_pow2(def->align));

#if __STDC_VERSION__ >= 201112L
    gboing_assert(gboing_is_pow2(_Alignof(max_align_t)));
#endif

    gboing_assert_msg(!(d.align & (d.align - 1)),
                      "align must be a power of two"); /* ??? */
    gboing_assert_msg(!(d.size % d.align),
                      "size must be a multiple of align");

    /* verify pbase is really aligned as advertised */
    gboing_assert_early(!((uintptr_t)pbase & (d.align - 1)));

    /* perform checks on buffer & buf_size */
    gboing_assert_early(buffer || !buf_size);
    gboing_assert_early(((uintptr_t)buffer
                        % gboing_max(d.align, PTR_ALIGN)) == 0);


    gboing_assert_msg(!d.aligned_alloc || !!d.free,
                      "aligned_alloc requires a free function");

    /* Allocate and/or calculate memory requirements */

    /* ==== d.elem_buf ==== */
    if (!d.elem_buf) {

        /* can we use the supplied buffer? */
        if (buf_size >= d.size) {
            d.elem_buf = buffer;
            buf_used += d.size;
            elem_buf_is_set = 1;

        /* can we put it on the stack? */
        } else if (d.size < d.max_stack) {
            stack_used += d.size;
            d.elem_buf = gboing_aligned_alloca(d.align, d.size);
            elem_buf_is_set = 1;

            /* This test fails! it may be because gcc is sometimes able to
             * determine that d.elem_buf isn't used and so never allocates
             * it, not sure yet. It succeeds if we explicitly use it.*/
            *((char*)d.elem_buf) = 0;
            gboing_assert(!!d.elem_buf);

        /* otherwise, it must go on the heap */
        } else {
            tmp_needed += d.size;
            tmp_align = d.align;
        }
    } else
        elem_buf_is_set = 1;

    /* ==== qsort node stack ==== */
    /* subtracting max_thresh needs to be tested & verified */
    qstack_size = sizeof(stack_node) * (d.max_size_bits - d.max_thresh + 1);

    /* keep properly aligned */
    pad_size = (buf_used % QSTACK_ALIGN)
             ? QSTACK_ALIGN - (buf_used % QSTACK_ALIGN)
             : 0;

    /* Can we put qstack on supplied buffer? */
    if (buf_size >= buf_used + pad_size + qstack_size) {

        buf_used += pad_size;
        qstack = buffer + buf_used;
        buf_used += qstack_size;

    /* Can we put it on the stack? */
    } else if (stack_used + qstack_size <= d.max_stack) {
        stack_used += qstack_size;
        qstack = gboing_aligned_alloca(QSTACK_ALIGN, qstack_size);

    /* otherwise, it must be on the heap */
    } else {
        /* if elem_buf is already going there, we need correct alignment */
        if (tmp_needed) {
            pad_size = (tmp_needed % QSTACK_ALIGN)
                       ? QSTACK_ALIGN - (tmp_needed % QSTACK_ALIGN)
                       : 0;
            tmp_needed += pad_size;
            qstack_tmp_offset = tmp_needed;     /* record offset */

            /* otherwise, we'll allocate it with qstack's alignment */
        } else
            tmp_align = QSTACK_ALIGN;

        tmp_needed += qstack_size;
    }

    gboing_assert_const(tmp_align);
    gboing_assert_const(tmp_needed);

    /* ==== indirection buffer ==== -- never goes on stack */
    if (indirect) {
        size_t index_size = sizeof(void *) * n;     /* rt value */

        /* try buffer first */
        pad_size = (buf_used % PTR_ALIGN)
                   ? PTR_ALIGN - (buf_used % PTR_ALIGN)
                   : 0;

        if (buf_size >= buf_used + pad_size + index_size) {
            buf_used += pad_size;
            d.index = buffer + buf_used;

        /* otherwise we'll allocate heap space */
        } else {
           if (tmp_needed) {
                /* keep properly aligned if other objects are going on this buffer */
                pad_size = (tmp_needed % PTR_ALIGN)
                           ? PTR_ALIGN - (tmp_needed % PTR_ALIGN)
                           : 0;
                tmp_needed += pad_size;

                index_tmp_offset = tmp_needed;
            } else {
                /* tmp_align becomes rt value here */
                tmp_align = PTR_ALIGN;
            }

            gboing_assert_const(tmp_needed);
            gboing_assert_const(index_tmp_offset);
            /* tmp_needed becomes a run-time value here due to index_size */
            tmp_needed += index_size;
        }
    }

    /* allocate if heap space needed */
    if (tmp_needed) {
        if (!!d.aligned_alloc)
            tmp_buffer = d.aligned_alloc(tmp_align, tmp_needed);
        else
            tmp_buffer = gboing_aligned_alloc(tmp_align, tmp_needed);


        if (!tmp_buffer)
            return ENOMEM;

        if (!d.elem_buf)
            d.elem_buf = tmp_buffer;
        else
            gboing_assert(elem_buf_is_set);

        if (!qstack)
            qstack = tmp_buffer + qstack_tmp_offset;

        if (indirect && !d.index)
            d.index = tmp_buffer + index_tmp_offset;
    }

    /* now as long as we haven't erred in any of our padding calculation, this
     * should never cause bad code generation*/
    d.elem_buf    = gboing_assume_aligned(d.elem_buf, d.align);
    qstack        = gboing_assume_aligned(qstack, QSTACK_ALIGN);
    d.index       = gboing_assume_aligned(d.index, PTR_ALIGN);

    /* but just to be safe, let's verify it for now */
    assert(!((uintptr_t)d.elem_buf % d.align));
    assert(!((uintptr_t)qstack % QSTACK_ALIGN));
    assert(!((uintptr_t)d.index % PTR_ALIGN));

    /* if using indirection, we'll now swap out d.size and d.align */
    if (indirect) {
        size_t i;

        d.size = sizeof(void *);
        d.align = PTR_ALIGN;

        assert(!((uintptr_t)d.index & (d.align - 1)));

        for (i = n; i--;)
            d.index[i] = base_ptr + i * def->size;

        base_ptr = (char *) d.index;

    }

    /* now that we have d.size figured out... */
    max_thresh = d.max_thresh * d.size;

    /* These locals should still be compile-time constants */
    gboing_assert_const(indirect);
    gboing_assert_const(max_thresh);
    gboing_assert_const(buf_used);       /* note any index size is omitted */
    gboing_assert_const(QSTACK_ALIGN);
    gboing_assert_const(PTR_ALIGN);
    gboing_assert_const(pad_size);
    gboing_assert_const(stack_used);
    gboing_assert_const(qstack_size);
    gboing_assert_const(qstack_tmp_offset);
    /* gboing_assert_const(index_tmp_offset); broken test!!! */


    /* ==== Merge sort ==== */
    if (n > d.max_thresh) {
        char *lo = base_ptr;
        char *hi = &lo[d.size * (n - 1)];
        stack_node *top = qstack;

        _qsort_push(&top, NULL, NULL);

        while (qstack < top) {
            char *left_ptr;
            char *right_ptr;

            /* Select median value from among LO, MID, and HI. Rearrange
               LO and HI so the three values are sorted. This lowers the
               probability of picking a pathological pivot value and
               skips a comparison for both the LEFT_PTR and RIGHT_PTR in
               the while loops. */

            char *mid = lo + d.size * ((hi - lo) / d.size >> 1);

            if (_qsort_less(&d, (void *) mid, (void *) lo, arg))
                _qsort_swap(&d, mid, lo);

            if (_qsort_less(&d, (void *) hi, (void *) mid, arg)) {
                _qsort_swap(&d, mid, hi);

                if (_qsort_less(&d, (void *) mid, (void *) lo, arg))
                    _qsort_swap(&d, mid, lo);
            }

            left_ptr  = lo + d.size;
            right_ptr = hi - d.size;

            /* Here's the famous ``collapse the walls'' section of qsort.
               Gotta like those tight inner loops!  They are the main reason
               that this algorithm runs much faster than others. */
            do {
              while (_qsort_less (&d, (void *) left_ptr, (void *) mid, arg))
                left_ptr += d.size;

              while (_qsort_less (&d, (void *) mid, (void *) right_ptr, arg))
                right_ptr -= d.size;

                if (left_ptr < right_ptr) {
                    _qsort_swap(&d, left_ptr, right_ptr);

                    if (mid == left_ptr)
                        mid = right_ptr;
                    else if (mid == right_ptr)
                        mid = left_ptr;

                    left_ptr += d.size;
                    right_ptr -= d.size;
                } else if (left_ptr == right_ptr) {
                    left_ptr += d.size;
                    right_ptr -= d.size;
                    break;
                }
            } while (left_ptr <= right_ptr);

            /* Set up pointers for next iteration.  First determine whether
               left and right partitions are below the threshold size.  If so,
               ignore one or both.  Otherwise, push the larger partition's
               bounds on the stack and continue sorting the smaller one. */

            if ((size_t)(right_ptr - lo) <= max_thresh) {
                if ((size_t)(hi - left_ptr) <= max_thresh)
                    /* Ignore both small partitions. */
                    _qsort_pop(&top, &lo, &hi);
                else
                    /* Ignore small left partition. */
                    lo = left_ptr;
            } else if ((size_t)(hi - left_ptr) <= max_thresh) {
                /* Ignore small right partition. */
                hi = right_ptr;
            } else if ((right_ptr - lo) > (hi - left_ptr)) {
                /* Push larger left partition indices. */
                _qsort_push(&top, lo, right_ptr);
                lo = left_ptr;
            } else {
                /* Push larger right partition indices. */
                _qsort_push(&top, left_ptr, hi);
                hi = right_ptr;
            }
        }
    }

    /* ==== Insertion sort ==== */

    /* Once the BASE_PTR array is partially sorted by the merge sort, the rest
     * is completely sorted using insertion sort, since this is efficient
     * for partitions below d.max_thresh size. BASE_PTR points to the beginning
     * of the array to sort, and END_PTR points at the very last element in
     * the array (*not* one beyond it!). */


    /* if element size is a power of two, indexed addressing will be more
     * efficient in most cases */
    if (gboing_is_pow2(d.size) && d.size <= _QSORT_ARCH_MAX_INDEX_MULT) {
        const size_t thresh = gboing_min(n, d.max_thresh + 1);
        size_t left, right;
        void *smallest;

        /* Find smallest element in first threshold and place it at the
         * array's beginning.  This is the smallest array element,
         * and the operation speeds up insertion sort's inner loop. */

        for (smallest = base_ptr, right = 1; right < thresh; ++right) {
            char *p = base_ptr + right * d.size;

            if (_qsort_less(&d, p, smallest, arg))
                smallest = p;
        }

        if (smallest != base_ptr)
            _qsort_swap(&d, smallest, base_ptr);

        for (right = 2; right < n; ++right) {
            left = right - 1;

            while (_qsort_less(&d, &base_ptr[right * d.size],
                                   &base_ptr[left  * d.size], arg)) {
                assert(left);
                --left;
            }

            ++left;

            if (left != right)
                _qsort_ror(&d, &base_ptr[left * d.size], &base_ptr[right * d.size]);
        }
    } else {
        /* if not a power of two, use ptr arithmetic */
        char *const end_ptr = &base_ptr[d.size * (n - 1)];
        char *tmp_ptr = base_ptr;
        char *thresh = gboing_min(end_ptr, base_ptr + max_thresh);
        register char *run_ptr;

        /* Find smallest element in first threshold and place it at the
           array's beginning.  This is the smallest array element,
           and the operation speeds up insertion sort's inner loop. */

        for (run_ptr = tmp_ptr + d.size; run_ptr <= thresh; run_ptr += d.size)
            if (_qsort_less(&d, (void *) run_ptr, (void *) tmp_ptr, arg))
                tmp_ptr = run_ptr;

        if (tmp_ptr != base_ptr)
            _qsort_swap(&d, tmp_ptr, base_ptr);

        /* Insertion sort, running from left-hand-side up to right-hand-side.  */

        run_ptr = base_ptr + d.size;

        while ((run_ptr += d.size) <= end_ptr) {
            tmp_ptr = run_ptr - d.size;

            while (_qsort_less(&d, (void *) run_ptr, (void *) tmp_ptr, arg))
                tmp_ptr -= d.size;

            tmp_ptr += d.size;

            if (tmp_ptr != run_ptr)
                _qsort_ror(&d, tmp_ptr, run_ptr);
        }
    }


    /* if we used indirect sorting, now we have to re-arrange the array. */
    if (indirect) {
        size_t i;     /* current index & element entries */
        char *ip;     /* pointer to the current element */
        char *kp;     /* current element at index[i] */
        void **index = d.index;

        d.size  = def->size;
        d.align = def->align;
        d.index = NULL;

        for (i = 0, ip = (char *)pbase; i < n; ++i, ip += d.size) {
            if ((kp = index[i]) != ip) {
                size_t j = i;
                char *jp = ip;
                _qsort_copy(&d, d.elem_buf, ip);

                do {
                    size_t k = (kp - (char *)pbase) / d.size;
                    index[j] = jp;
                    _qsort_copy(&d, jp, kp);
                    j = k;
                    jp = kp;
                    kp = index[k];
                } while (kp != ip);

                index[j] = jp;
                _qsort_copy(&d, jp, d.elem_buf);
            }
        }
    }

    if (tmp_needed) {
        if (d.free)
            d.free(tmp_buffer);
        else
            /* TODO: make sure this works when alignment of allocated buffer was
             * performed */
            gboing_aligned_free(tmp_buffer);
    }

    return 0;
}
#endif /* __GNUC_PREREQ (4, 8) */
#endif /* _QSORT_TEMPLATE_H_ */

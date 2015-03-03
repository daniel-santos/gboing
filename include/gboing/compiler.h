/*
 * compiler.h --
 * Copyright (C) 2014, 2015 Daniel Santos <daniel.santos@pobox.com>
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

/** @file */

#ifndef _GBOING_COMPILER_H_
#define _GBOING_COMPILER_H_

#include <gboing/cpp.h>

#ifdef __GNUC__
# include <gboing/compiler-gcc.h>
#endif

/* icc defines __GNUC__, so this header should override as needed */
#ifdef __INTEL_COMPILER
# include <gboing/compiler-intel.h>
#endif

/* clang defines __GNUC__, so this header should override as needed */
#ifdef __clang__
# include <gboing/compiler-clang.h>
#endif

#ifdef __HP_aCC
# include <gboing/compiler-aCC.h>
#endif

#ifdef _MSC_VER
# include <gboing/compiler-msvc.h>
#endif



/**
 * gboing_assert_msg - break build and emit msg if expr is false
 * @param expr: a compile-time constant expr to check
 * @param msg:       a message to emit if expr is false
 *
 * This macro will break the build if the expr evaluates to zero (false),
 * emitting the supplied error message if the compiler has support to do so.
 */
#define gboing_assert_msg(expr, msg) \
	_gboing_check(expr, msg, error, __gboing_assert_, __LINE__)

#define gboing_assert(expr) \
	gboing_assert_msg(expr, "Assertion failed: " #expr)

/**
 * static_warn_msg - emit a warming msg if expr is false
 * @param expr: a compile-time constant expr to check
 * @param msg:       a message to emit if expr is false
 */
#define gboing_warn_msg(expr, msg) \
	_gboing_static_check(!(expr), msg, warning, __static_warn_, __LINE__)

#define gboing_warn(expr) \
	gboing_warn_msg(!(expr), "Warning: " #expr)


/**
 * gboing_assert_power_of_2 -- Verify that an expression represents a power of
 * two. */
#define gboing_assert_power_of_2(expr)			\
    gboing_assert_msg(({				\
		typeof (expr) _n = (expr);		\
		_n && !(_n & (_n - 1))			\
	}), "Not a power of two: " #expr)

/**
 * assert_const -- Verify that an expression can be resolved to a constant
 * value at compilation time during an optimized build.
 */
#define gboing_assert_const(expr)			\
	gboing_assert_msg(__builtin_constant_p(expr),	\
		      "Not a constant expression: " #expr)

/**
 * assert_early -- If expr is compile-time constant, perform a static
 * assertion. Otherwise, verify the value at run-time with a traditional
 * assert().
 */
#define gboing_assert_early(expr)			\
    do {						\
        if (__builtin_constant_p(expr))			\
            gboing_assert(expr);			\
        else						\
            assert(expr);				\
    } while (0)



#ifndef _gboing_check
# define _gboing_check(expr, msg, errwarn, prefix, suffix) \
						do {} while(0)
#endif
#ifndef gboing_aligned
# define gboing_aligned(n)
#endif
#ifndef gboing_aligned_alloca
# define gboing_aligned_alloca(n, align)	alloca(n)
#endif
#ifndef gboing_alloc_size
# define gboing_alloc_size(...)
#endif
#ifndef gboing_always_inline
# define gboing_always_inline			inline
#endif
#ifndef gboing_assume_aligned
# define gboing_assume_aligned(p, ...)		(p)
#endif
#ifndef gboing_cold
# define gboing_cold
#endif
#ifndef gboing_compiler_offsetof
# include <stddef.h>
# define gboing_compiler_offsetof(a, b)		offsetof(a, b)
#endif
#ifndef gboing_const
# define gboing_const
#endif
#ifndef gboing_constructor
# define gboing_constructor
#endif
#ifndef gboing_deprecated
# define gboing_deprecated
#endif
#ifndef gboing_deprecated_msg
# define gboing_deprecated_msg(msg)
#endif
#ifndef gboing_destructor
# define gboing_destructor
#endif
#ifndef gboing_expect
# define gboing_expect(a, b)			(a)
#endif
#ifndef gboing_flatten
# define gboing_flatten
#endif
#ifndef gboing_bswap16
//# define gboing_bswap16(expr)
#endif
#ifndef gboing_bswap32
//# define gboing_bswap32(expr)
#endif
#ifndef gboing_bswap64
//# define gboing_bswap64(expr)
#endif
#ifndef gboing_hot
# define gboing_hot
#endif
#ifndef gboing_likely
# define gboing_likely(expr)			(expr)
#endif
#ifndef gboing_malloc
# define gboing_malloc
#endif
#ifndef gboing_must_check
# define gboing_must_check
#endif
#ifndef gboing_noclone
# define gboing_noclone
#endif
#ifndef gboing_noinline
# define gboing_noinline
#endif
#ifndef gboing_noreturn
# define gboing_noreturn
#endif
#ifndef gboing_packed
//# define gboing_packed
#endif
#ifndef gboing_pure
# define gboing_pure
#endif
#ifndef gboing_regparm
# define gboing_regparm
#endif
#ifndef gboing_section
//# define gboing_section
#endif
#ifndef gboing_unlikely
# define gboing_unlikely(expr)			(expr)
#endif
#ifndef gboing_unreachable
# define gboing_unreachable()			do {} while(0)
#endif
#ifndef gboing_used
# define gboing_used
#endif
#ifndef gboing_visible
# define gboing_visible
#endif
#ifndef gboing_weak
# define gboing_weak
#endif

/* For lack of a better mechanism right now */
#define assume_aligned	gboing_assume_aligned
#define __noinline	gboing_noinline
#define __flatten	gboing_flatten
#define assert_early	gboing_assert_early
#define assert_const	gboing_assert_const
#define likely		gboing_likely
#define unlikely	gboing_unlikely

#endif /* _GBOING_COMPILER_H_ */

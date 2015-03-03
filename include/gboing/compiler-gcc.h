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

#ifndef _GBOING_COMPILER_GCC_H
#define _GBOING_COMPILER_GCC_H

#include <sys/types.h>
#include <inttypes.h>

#define GBOING_HAVE_CPP_ARITH

#ifndef __GNUC_PATCHLEVEL__
# error Compiler too old
#endif

/**
 * @defgroup macros Preprocessor macros
 *
 * @{
 */

#define GCC_VERSION (__GNUC__ * 10000 \
		   + __GNUC_MINOR__ * 100 \
		   + __GNUC_PATCHLEVEL__)

#if GCC_VERSION < 29600
# error Compiler too old
#endif


#define gboing_max(a, b) ({	\
	typeof (a) _a = (a);	\
	typeof (b) _b = (b);	\
	_a > _b ? _a : _b;	\
})

# define gboing_min(a, b) ({	\
	typeof (a) _a = (a);	\
	typeof (b) _b = (b);	\
	_a < _b ? _a : _b;	\
})

#define gboing_typeof(expr)		typeof(expr)
#define gboing_always_inline		inline __attribute__((always_inline))
#define gboing_noinline			__attribute__((noinline))
#define gboing_noreturn			__attribute__((noreturn))
#define gboing_pure			__attribute__((pure))
#define gboing_const			__attribute__((const))
#define gboing_section(name)		__attribute__((section(name)))
#define gboing_constructor		__attribute__((constructor))
#define gboing_destructor		__attribute__((destructor))
#define gboing_malloc			__attribute__((malloc))
#define gboing_alloc_size(...)		__attribute__((alloc_size(__VA_ARGS__)))
#define gboing_likely(x)		__builtin_expect(!!(x), 1)
#define gboing_unlikely(x)		__builtin_expect(!!(x), 0)
#define gboing_expect(a, b)		__builtin_expect(!!(a), !!(b))
#define gboing_aligned(n)		__attribute__((aligned(n)))
#define gboing_packed			__attribute__((packed))



#if defined(__x86_64) || defined(__i386)
# define gboing_regparm(n)		__attribute__((regparm(n)))
#endif



#if GCC_VERSION > 30100
# define gboing_deprecated		__attribute__((deprecated))
# define gboing_deprecated_msg(msg)	__attribute__((deprecated(msg)))

/* Introduced in 3.1, but it was named unused until 3.3 */
# if GCC_VERSION < 30300
#  define gboing_used			__attribute__((__unused__))
# else
#  define gboing_used			__attribute__((__used__))
# endif
#endif

#if GCC_VERSION >= 30400
# define gboing_must_check		__attribute__((warn_unused_result))
#endif

/* weak was miscompiled from 4.1 to 4.1.1 */
#if (GCC_VERSION < 40100 || GCC_VERSION > 40101)
# define gboing_weak			__attribute__((weak))
#endif

#if GCC_VERSION >= 40000
# define gboing_compiler_offsetof(a, b)	__builtin_offsetof(a, b)
#endif

#if GCC_VERSION >= 40100
# define gboing_flatten			__attribute__((flatten))
#endif

#if GCC_VERSION >= 40300
# define gboing_cold			__attribute__((__cold__))
# define gboing_hot			__attribute__((__hot__))

/* _static_check doesn't work when not optimized */
# ifndef __OPTIMIZE__
#  define _gboing_check(expr, msg, errwarn, prefix, suffix) \
    do {} while (0)
# else
#  define _gboing_check(expr, msg, errwarn, prefix, suffix) \
	GBOING_EXPAND_ARGS(_gboing_check2, expr, msg, errwarn, prefix, suffix)

#  define _gboing_check2(expr, msg, errwarn, prefix, suffix)		\
    do {								\
        extern void prefix ## suffix(void) __attribute__((errwarn(msg))); \
        if (!(expr))							\
            prefix ## suffix();						\
    } while (0)

# endif /* __OPTIMIZE__ */
#endif /* GCC_VERSION >= 40300 */

#if GCC_VERSION >= 40400
# define gboing_bswap32(expr)		__builtin_bswap32(expr)
# define gboing_bswap64(expr)		__builtin_bswap64(expr)
# if GCC_VERSION >= 40800 || (defined(__powerpc__) && GCC_VERSION >= 40600)
#  define gboing_bswap16(expr)		__builtin_bswap16(expr)
# endif
#endif /* GCC_VERSION >= 40400 */

#if GCC_VERSION >= 40500
# define gboing_unreachable()		__builtin_unreachable()
# define gboing_noclone			__attribute__((__noclone__))
#endif /* GCC_VERSION >= 40500 */

#if GCC_VERSION >= 40600
# define gboing_visible			__attribute__((externally_visible))
#endif


/**
 * assume_aligned gives the compiler information about the alignment guarantees
 * of a pointer and has two forms. The first form accepts the pointer as the
 * first argument and an intergal value specifying the pointer's minimum
 * alignment as the second. The second form accepts a third integral argument
 * specifying a misalignment offset.
 *
 * Using this macro with a pointer who's alignment at run-time is not as
 * specified is likely to result in undefined behavior, sigbus, chaotic evil,
 * etc.
 *
 * @return the supplied pointer
 */
#if GCC_VERSION >= 40700
# define gboing_assume_aligned(p, ...) \
	((typeof (*p)*) __builtin_assume_aligned(p, ## __VA_ARGS__))
#endif



/**
 * aligned_alloca -- Allocate aligned memory on the stack.
 */
#define gboing_aligned_alloca(n, align) (void *)		\
    ({								\
        const size_t a = align - 1;				\
        return (void *) (((uintptr_t)alloca((n) + a) + a) & ~a);\
    })

/**
 * }@
 */

#endif /* _GBOING_COMPILER_GCC_H */

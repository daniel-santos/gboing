/**
 * @file compiler.h
 * @breif macros to encapsulate compiler-specific extensions
 */

/* Copyright (C) 2014, 2015 Daniel Santos <daniel.santos@pobox.com>
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

#ifndef _GBOING_COMPILER_H_
#define _GBOING_COMPILER_H_

#include <stdint.h>
#include <features.h>

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

#if 1
# define GBOING_HAVE_CPP_VARGS
#endif


#ifndef gboing_compiler_offsetof
# include <stddef.h>
# define gboing_compiler_offsetof(a, b)     offsetof(a, b)
#endif

/* Where should these utility macros go? */
/* TODO: add c11-style macro? */
#define gboing_is_pow2(x) (((x) & ((x) - 1)) == 0)
#ifndef gboing_min
# define gboing_min(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef gboing_max
# define gboing_max(a, b) ((a) > (b) ? (a) : (b))
#endif

/* Attributes for functions & data */

#ifndef gboing_always_inline
# define gboing_always_inline               inline
#endif
#ifndef gboing_assume_aligned
# define gboing_assume_aligned(p, ...)      (p)
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
# define gboing_expect(a, b)                (a)
#endif
#ifndef gboing_flatten
# define gboing_flatten
#endif
#ifndef gboing_hot
# define gboing_hot
#endif
#ifndef gboing_cold
# define gboing_cold
#endif
#ifndef gboing_likely
# define gboing_likely(expr)                (expr)
#endif
#ifndef gboing_unlikely
# define gboing_unlikely(expr)              (expr)
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
#ifndef gboing_unreachable
# define gboing_unreachable()           do {} while(0)
#endif
#ifndef gboing_used
# define gboing_used
#endif
#ifndef gboing_unused
# define gboing_unused
#endif
#ifndef gboing_visible
# define gboing_visible
#endif
#ifndef gboing_weak
# define gboing_weak
#endif

#endif /* _GBOING_COMPILER_H_ */

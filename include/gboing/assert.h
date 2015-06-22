/**
 * @file assert.h
 * @breif alternative mechanisms for run- and compile-time assertions
 */

/* Copyright (C) 2013-2015 Daniel Santos <daniel.santos@pobox.com>
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

#ifndef _GBOING_ASSERT_H_
#define _GBOING_ASSERT_H_

#include <assert.h>
#include <gboing/compiler.h>
#include <gboing/cpp.h>

/**
 * @defgroup macros Preprocessor macros
 *
 * @{
 */

/**
 * @def gboing_assert_msg(expr, msg)
 * @brief break build and emit msg if expr is false
 * @param expr a compile-time constant expression
 * @param msg a message to emit if expr is false
 *
 * Break the build if the expr evaluates to zero (false), emitting the supplied
 * error message if the compiler has support to do so. If the compiler has no
 * such support (not gcc or built -O0) does it does nothing. If will always
 * fail expr is not a compile-time constant.
 */
#define gboing_assert_msg(expr, msg) \
    _gboing_check(expr, msg, error, __gboing_assert_, __LINE__)

#define gboing_assert(expr) \
    gboing_assert_msg(expr, "Assertion failed: " #expr)

/**
 * @def static_warn_msg(expr, msg)
 * @brief emit a warming msg if expr is false
 * @param expr a compile-time constant expression
 * @param msg a message to emit if expr is false
 *
 * BUG: currently results in linktime-failure.
 */
#define gboing_warn_msg(expr, msg) \
    _gboing_static_check(!(expr), msg, warning, __static_warn_, __LINE__)

#define gboing_warn(expr) \
    gboing_warn_msg(!(expr), "Warning: " #expr)


/**
 * @def gboing_assert_pow2(expr)
 * @brief Verify that a compile-time constant expression represents a power of
 * @param expr a compile-time constant expression
 */
#define gboing_assert_pow2(expr)        \
    gboing_assert_msg(({                \
        typeof (expr) _n = (expr);      \
        _n && gboing_is_pow2(n)         \
    }), "Not a power of two: " #expr)

/**
 * @def gboing_assert_pow2(expr)
 * @brief Verify that an expression is a compile-time constant
 * @param expr an expression
 *
 * Breaks this build if __builtin_constant_p(expr) returns zero.
 */
#define gboing_assert_const(expr)                           \
    gboing_assert_msg(__builtin_constant_p(expr),           \
                      "Not a constant expression: " #expr)

/**
 * @def gboing_assert_early(expr)
 * @brief Perform earliest possible assertion type
 * @param expr an expression (either runtime or compile-time constant)
 *
 * If expr is compile-time constant then this macro perform a static assertion
 * -- if not it verifies the value at run-time with a traditional assert().
 */
#define gboing_assert_early(expr)       \
    do {                                \
        if (__builtin_constant_p(expr)) \
            gboing_assert(expr);        \
        else                            \
            assert(expr);               \
    } while (0)

#ifndef _gboing_check
# define _gboing_check(expr, msg, errwarn, prefix, suffix) \
         do {} while(0)
#endif

/**
 * @}
 */

#endif /* _GBOING_ASSERT_H_ */

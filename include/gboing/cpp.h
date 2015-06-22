/**
 * @file
 * @breif preprocessor malice and trickery!
 */

/* Copyright (C) 2012-2015 Daniel Santos <daniel.santos@pobox.com>
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

#ifndef _GBOING_CPP_H_
#define _GBOING_CPP_H_

/**
 * @defgroup macros Preprocessor macros
 *
 * @{
 */

#ifdef GBOING_HAVE_CPP_VARGS
/**
 * @def GBOING_EXPAND_ARGS(args)
 * @brief A cheap trick to expand macro arguments and call another macro
 * @param next the next macro to call (expand) or an actual function expression
 * @param args arguments to pass to that next macro
 *
 * NOTE: cpp doesn't do recursion, thus there are 10 identical versions of this
 * macro with different names when needed more than once within an expansion
 * unit.
 */
# define GBOING_EXPAND_ARGS(next, ...) next(__VA_ARGS__)
# define GBOING_EXPAND_ARGS2(next, ...) next(__VA_ARGS__)
# define GBOING_EXPAND_ARGS3(next, ...) next(__VA_ARGS__)
# define GBOING_EXPAND_ARGS4(next, ...) next(__VA_ARGS__)
# define GBOING_EXPAND_ARGS5(next, ...) next(__VA_ARGS__)
# define GBOING_EXPAND_ARGS6(next, ...) next(__VA_ARGS__)
# define GBOING_EXPAND_ARGS7(next, ...) next(__VA_ARGS__)
# define GBOING_EXPAND_ARGS8(next, ...) next(__VA_ARGS__)
# define GBOING_EXPAND_ARGS9(next, ...) next(__VA_ARGS__)

# ifdef GBOING_HAVE_CPP_OMIT_COMMA

/**
 * @def GBOING_NARGS(args)
 * @brief Returns the number of arguments supplied up to a maximum of 16
 */
#  define GBOING_NARGS(...) _GBOING_NARGS_SEQ(__VA_ARGS__, \
              16,  15,  14,  13,  12,  11,  10,  9, \
              8,   7,   6,   5,   4,   3,   2,   1)
#define _GBOING_NARGS_SEQ(_1,  _2,  _3,  _4,  _5,  _6,  _7,  _8, \
              _9,  _10, _11, _12, _13, _14, _15, _16, N, ...) N

/**
 * @def GBOING_IFF_EMPTY(test, t, f)
 * @brief Returns t if test is defined, f otherwise
 * @param test a preprocessor expression (macro) that may or may not be empty
 * @param t result if true
 * @param f result if false
 *
 * Relying on gcc's extensions that allows the omission of a comma for an empty
 * argument, we can cobble together an intermediate if empty.
 */
#  define GBOING_IFF_EMPTY(test, t, f) \
    _GBOING_IFF_EMPTY2(_GBOING_JUNK##test, t, f)
#  define _GBOING_JUNK junk,
#  define _GBOING_IFF_EMPTY2(test_or_junk, t, f) \
    _GBOING_IFF_EMPTY3(test_or_junk, t, f)
#  define _GBOING_IFF_EMPTY3(__ignored1, __ignored2, result, ...) result
# endif /* GBOING_HAVE_CPP_OMIT_COMMA */
#endif /* GBOING_HAVE_CPP_VARGS */

/**
 * @def GBOING_CAT(a, b)
 * @brief A stupid concatination macro
 */
#define GBOING_CAT(a, b) _GBOING_CAT(a, b)
#define _GBOING_CAT(a, b) a ## b

/**
 * @def GBOING_CAT3(a, b, c)
 * @brief Concatinates 3 arguments after allowing them to expand
 */
#define GBOING_CAT3(a, b, c) _GBOING_CAT3(a, b, c)
#define _GBOING_CAT3(a, b, c) a ## b ## c

/**
 * @def GBOING_CAT4(a, b, c, d)
 * @brief Concatinates 4 arguments after allowing them to expand
 */
#define GBOING_CAT4(a, b, c, d) _GBOING_CAT4(a, b, c, d)
#define _GBOING_CAT4(a, b, c, d) a ## b ## c ## d

/**
 * @def GBOING_STRIZE(expr)
 * @brief Stringifies the given expression after allowing it to expand
 */
#define GBOING_STRIZE(expr) _GBOING_STRIZE(expr)
#define _GBOING_STRIZE(expr) #expr

#endif /* _GBOING_CPP_H_ */

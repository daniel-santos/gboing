/*
 * cpp.h --
 * Copyright (C) 2013-2015 Daniel Santos <daniel.santos@pobox.com>
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

#define GBOING_EXPAND_ARGS(next, ...) next(__VA_ARGS__)
#define GBOING_EXPAND_ARGS2(next, ...) next(__VA_ARGS__)
#define GBOING_EXPAND_ARGS3(next, ...) next(__VA_ARGS__)
#define GBOING_EXPAND_ARGS4(next, ...) next(__VA_ARGS__)
#define GBOING_EXPAND_ARGS5(next, ...) next(__VA_ARGS__)
#define GBOING_EXPAND_ARGS6(next, ...) next(__VA_ARGS__)
#define GBOING_EXPAND_ARGS7(next, ...) next(__VA_ARGS__)
#define GBOING_EXPAND_ARGS8(next, ...) next(__VA_ARGS__)
#define GBOING_EXPAND_ARGS9(next, ...) next(__VA_ARGS__)

#define _GBOING_JUNK junk,
#define _GBOING_IFF_EMPTY2(test_or_junk, t, f) \
	_GBOING_IFF_EMPTY3(test_or_junk, t, f)
#define _GBOING_IFF_EMPTY3(__ignored1, __ignored2, result, ...) result

#define GBOING_IFF_EMPTY(test, t, f) \
	_GBOING_IFF_EMPTY2(_GBOING_JUNK##test, t, f)


#define _GBOING_NARGS_SEQ(_1,  _2,  _3,  _4,  _5,  _6,  _7,  _8, \
			  _9,  _10, _11, _12, _13, _14, _15, _16, N, ...) N

/* This counts the number of args up to a maximum of 16 */
#define GBOING_NARGS(...) _GBOING_NARGS_SEQ(__VA_ARGS__, \
			  16,  15,  14,  13,  12,  11,  10,  9, \
			  8,   7,   6,   5,   4,   3,   2,   1)

/* This will let macros expand before concating them */
#define _GBOING_PRIMITIVE_CAT(x, y) x ## y
#define GBOING_CAT(x, y) _GBOING_PRIMITIVE_CAT(x, y)

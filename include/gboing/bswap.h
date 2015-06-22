/**
 * @file bswap.h
 * @breif byte swap abstraction / implementation
 */

/* Copyright (C) 2015 Daniel Santos <daniel.santos@pobox.com>
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

#ifndef _GBOING_BSWAP_H_
#define _GBOING_BSWAP_H_

#include <stdint.h>
#include <gboing/compiler.h>

#ifndef gboing_bswap16
# define gboing_bswap16(expr) _gboing_bswap16(expr)
static gboing_const gboing_unused uint16_t _gboing_bswap16(uint16_t x) {
    return ((x & 0xff00u) >> 8)
         | ((x & 0x00ffu) << 8);
}
#endif /* gboing_bswap16 */

#ifndef gboing_bswap32
# define gboing_bswap32(expr) _gboing_bswap32(expr)
static gboing_const gboing_unused uint32_t _gboing_bswap32(uint32_t x) {
    return ((x & 0xff000000u) >> 24)
         | ((x & 0x00ff0000u) >> 8)
         | ((x & 0x0000ff00u) << 8)
         | ((x & 0x000000ffu) << 24);
}
#endif /* gboing_bswap32 */

#ifndef gboing_bswap64
# define gboing_bswap64(expr) _gboing_bswap64(expr)
static gboing_const gboing_unused uint64_t _gboing_bswap64(uint64_t x) {
    return ((x & 0xff00000000000000ull) >> 56)
         | ((x & 0x00ff000000000000ull) >> 40)
         | ((x & 0x0000ff0000000000ull) >> 24)
         | ((x & 0x000000ff00000000ull) >> 8)
         | ((x & 0x00000000ff000000ull) << 8)
         | ((x & 0x0000000000ff0000ull) << 24)
         | ((x & 0x000000000000ff00ull) << 40)
         | ((x & 0x00000000000000ffull) << 56);
}
#endif /* gboing_bswap64 */

#endif /* _GBOING_BSWAP_H_ */

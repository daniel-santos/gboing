/*
 * static_strlen - demo for qsort implemented as a C pseudo-template function
 * Copyright (C) 2014 Daniel Santos <daniel.santos@pobox.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */

#define  _ISOC11_SOURCE
#define _GNU_SOURCE

#include "gboing/qsort.h"

#ifndef ELEM_SIZE
# define ELEM_SIZE 4096
#endif

#ifndef ALIGN_SIZE
# define ALIGN_SIZE 4
#endif

#ifndef KEY_SIGN
# define KEY_SIGN uint
#endif

#include "test-common.h"


#define key_type(bits) KEY_SIGN ## bits ## _t

/* packing isn't normally something one would do, but we need to force the
 * specified object size, so we use pack(1) to get that */
#pragma pack(push,1)
struct size_type {
	char c[ELEM_SIZE];
};
#pragma pack(pop)

static const struct qsort_def my_def = {
	.size   = ELEM_SIZE,
	.align  = ALIGN_SIZE,
#ifdef USE_LESS
	.less   = my_less,
#else
	.compar = my_cmp,
#endif
};


__flatten void my_quicksort(void *p, size_t n, size_t elem_size, int (*compar)(const void *, const void *, void *arg), void *arg) {
	qsort_template(&my_def, p, n, NULL);
}

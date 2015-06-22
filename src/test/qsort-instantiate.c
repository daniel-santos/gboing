/*
 * qsort-instantiate.c - instantiates a qsort_template function, used for
 *                       measuring generated function sizes in tests
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

#include "qsort-common.h"

gboing_noinline gboing_flatten void
my_quicksort(void *p, size_t n, size_t elem_size, compar_t compar, void *arg) {
    qsort_template(&my_def, NULL, 0, p, n, NULL);
}

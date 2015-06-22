/*
 * test-common.h -- Some utility macros and functions lazily put into static form
 * Copyright (C) 2015 Daniel Santos <daniel.santos@pobox.com>
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

#ifndef _UTILS_H_
#define _UTILS_H_

#include <error.h>
#include <errno.h>
#include <assert.h>

//#include "gboing/compiler.h"


#if 1
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <assert.h>
#endif

#define _do_error(fatal, err, file, line, fmt, ...)         \
    error_at_line(fatal, err, file, line, "%s - " fmt,      \
              __PRETTY_FUNCTION__, ## __VA_ARGS__)
#define fatal_error(fmt, ...)                       \
    _do_error(1, errno, __FILE__, __LINE__, fmt, ## __VA_ARGS__)



static inline void timespec_set(struct timespec *ts) {
	if (gboing_unlikely(errno = clock_gettime(CLOCK_THREAD_CPUTIME_ID, ts)))
		fatal_error("clock_gettime");
}

static inline struct timespec timespec_subtract(struct timespec a, struct timespec b) {
	const long ONE_BILLION = 1000000000ul;
	struct timespec ret = {
		.tv_sec  = a.tv_sec  - b.tv_sec,
		.tv_nsec = a.tv_nsec - b.tv_nsec,
	};

	if (ret.tv_nsec < 0) {
		ret.tv_sec  -= 1;
		ret.tv_nsec += ONE_BILLION;
	}

	return ret;
}

static inline struct timespec timespec_add(const struct timespec a, const struct timespec b) {
	const long ONE_BILLION = 1000000000ul;
	struct timespec ret = {
		.tv_sec  = a.tv_sec  + b.tv_sec,
		.tv_nsec = a.tv_nsec + b.tv_nsec,
	};

	if (ret.tv_nsec >= ONE_BILLION) {
		ret.tv_sec  += 1;
		ret.tv_nsec -= ONE_BILLION;
	}

	return ret;
}

static gboing_unused double timespec_pct(const struct timespec *a, const struct timespec *b) {
	const double ONE_BILLION = 1000000000.;
	double da = (double)a->tv_nsec + ONE_BILLION *a->tv_sec;
	double db = (double)b->tv_nsec + ONE_BILLION *b->tv_sec;
	return ((da / db) - 1.) * 100.;
}

static gboing_unused int timespec_lt(const struct timespec *a, const struct timespec *b) {
    if (a->tv_sec < b->tv_sec)
        return 1;
    else if (a->tv_sec > b->tv_sec)
        return 0;
    else
        return a->tv_nsec < b->tv_nsec;
}

#endif /* _UTILS_H_ */

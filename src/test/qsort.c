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

#include <stdalign.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <error.h>
#include <assert.h>
#include <getopt.h>
#include <unistd.h>

#include "gboing/qsort-template.h"

/* GNU's mqsort implementation. We have to define _GNU_SOURCE prior to
 * including stddef.h and include their qsort.c in the project for this to
 * link */
extern void _quicksort (void *const pbase, size_t total_elems, size_t size,
                        __compar_d_fn_t cmp, void *arg);



#include "test-common.h"
#include "qsort-common.h"

//#define key_type(bits) KEY_SIGN ## bits ## _t


static int verbose = 0;
static struct timespec max_time = {1, 0};
static size_t max_iterations = 0;
static size_t elem_count = 0;
static size_t data_size = 0;
static const double ONE_BILLION = 1000000000.;
static const char *argv0;

struct test_result {
    size_t count;
    struct timespec time;
    double ips;          /* iterations per second */
};


/* using static noinline to make it easier to examine generated code */
static gboing_noinline gboing_flatten void
my_quicksort(void *p, size_t n, size_t elem_size, compar_t compar, void *arg) {
    void *buffer = NULL;
    size_t buf_size = 0;
    size_t buf_align = gboing_max(my_def.align, gboing_alignof(void*));

    if (0) {
        buf_size = 0x10000;
        buffer = gboing_aligned_alloc(buf_align, buf_size);
        if (!buffer)
            fatal_error("malloc failed returned");
    }

    int ret = qsort_template(&my_def, buffer, buf_size, p, n, NULL);

    if (ret)
        fatal_error("qsort_template returned %d\n", ret);
}

static void dump_keys(void * const data[4], size_t n, const char *heading) {
    size_t i;

    fprintf(stderr, "\n%s:\n", heading);
    if (ELEM_SIZE == 1) {
        const uint8_t **p = (const uint8_t **)data;

        fprintf(stderr, "%-9s %-5s %-5s %-5s %-5s\n",
                "offset", "orig", "mine", "qsort", "msort");
        for (i = 0; i < n; ++i)
            fprintf(stderr, "%08lx: %02hhx    %02hhx    %02hhx    %02hhx\n",
                    i * sizeof(**p),
                    p[0][i], p[1][i], p[2][i], p[3][i]);
    } else if (ELEM_SIZE == 2) {
        const uint16_t **p = (const uint16_t **)data;

        fprintf(stderr, "%-9s %-5s %-5s %-5s %-5s\n",
                "offset", "orig", "mine", "qsort", "msort");
        for (i = 0; i < n; ++i)
            fprintf(stderr, "%08lx: %04hx  %04hx  %04hx  %04hx\n",
                    i * sizeof(**p),
                    p[0][i], p[1][i], p[2][i], p[3][i]);
    } else if (ELEM_SIZE == 4) {
        const uint32_t **p = (const uint32_t **)data;

        fprintf(stderr, "%-9s %-8s %-8s %-8s %-8s\n",
                "offset", "orig", "mine", "qsort", "msort");
        for (i = 0; i < n; ++i)
            fprintf(stderr, "%08lx: %08x %08x %08x %08x\n",
                    i * sizeof(**p),
                    p[0][i], p[1][i], p[2][i], p[3][i]);
    } else if (ELEM_SIZE >= 8) {
        const char **p = (const char **)data;

        fprintf(stderr, "%-9s %-16s %-16s %-16s %-16s\n",
                "offset", "orig", "mine", "qsort", "msort");
        for (i = 0; i < n; ++i)
            fprintf(stderr, "%08lx: %016lx %016lx %016lx %016lx\n",
                    i * sizeof(**p),
                    *(uint64_t*)(&p[0][i * ELEM_SIZE]),
                    *(uint64_t*)(&p[1][i * ELEM_SIZE]),
                    *(uint64_t*)(&p[2][i * ELEM_SIZE]),
                    *(uint64_t*)(&p[3][i * ELEM_SIZE]));
    }
}

/* Make sure that _quicksort_template() is correct given these parameters */
void validate_sort(size_t n, size_t elem_size, size_t min_align, unsigned int seed) {
    void *data[4];
    const char *algo_desc[4] = {"orig", "my_quicksort", "_quicksoft", "qsort_r"};
    const size_t DATA_SIZE = sizeof(data) / sizeof(*data);
    size_t bytes = n * elem_size;
    size_t i;

    gboing_assert(sizeof(struct size_type) == ELEM_SIZE);

    /* allocate buffers */
    for (i = 0; i < DATA_SIZE; ++i) {
        data[i] = aligned_alloc(min_align, bytes);
        assert(!((uintptr_t)data[i] & (min_align - 1)));
    }

    /* randomize first buffer */
    randomize(data[0], n, elem_size, seed);

    /* and copy to the other buffers */
    for (i = 1; i < DATA_SIZE; ++i) {
        memcpy(data[i], data[0], bytes);
        assert(!memcmp(data[i], data[0], bytes));
    }

    if (0)
        dump_keys(data[0], n, "BEFORE");

    my_quicksort(data[1], n, elem_size, NULL, NULL);
    _quicksort  (data[2], n, elem_size, my_compar_r, NULL);
    qsort_r     (data[3], n, elem_size, my_compar_r, NULL);

#if 0
    /* un-comment to induce error */
    *((int*)data[1]) -= 1;
#endif

    /* compare result of my_quicksort against results of other algos */
    for (i = 2; i < DATA_SIZE - 1; ++i) {
        if (memcmp(data[1], data[i], bytes)) {
            dump_keys(data, n, "");
            fprintf(stderr, "\n");
            fatal_error("\nmy_quicksort produced different result than %s", algo_desc[i]);
        }
    }

    for (i = 1; i < DATA_SIZE; ++i)
        free (data[i]);
}

static void print_result(struct test_result *res, const char *desc) {
	fprintf(stderr, "%16s = %12.6f iteraions per second (count=%lu, time=%02lu:%02lu.%09lu)\n",
            desc, res->ips, res->count, res->time.tv_sec / 60, res->time.tv_sec % 60, res->time.tv_nsec);
}

struct test_result run_test(void *p, unsigned int seed, sort_func_t sortfn, const char *desc) {
    const size_t n = elem_count;
    const size_t elem_size = ELEM_SIZE;
    const size_t min_align = ALIGN_SIZE;

    struct timespec start, end;
    struct test_result ret = {
        0,
        {0, 0},
        0.
    };
    size_t i;
    double dtime;

    gboing_assert_early(!(((uintptr_t)p) & (min_align - 1)));

#if 0
    printf("\n\nRunning test %s:\n"
        "n            = %lu\n"
        "elem_size    = %lu\n"
        "min_align    = %lu\n"
        "total bytes  = %lu\n"
        "repeat_count = %u\n",
        desc, n, elem_size, min_align, n * elem_size, max_iterations);
#endif

    srandom(seed);
    for (i = 0; i < max_iterations || !max_iterations; ++i) {
        randomize(p, n, elem_size, random());
        timespec_set(&start);

        sortfn(p, n, elem_size, my_compar_r, NULL);

        timespec_set(&end);
        ret.time = timespec_add(ret.time, timespec_subtract(end, start));

        if ((max_time.tv_sec | max_time.tv_nsec)
                && !timespec_lt(&ret.time, &max_time))
            break;
    }

    ret.count = i;
    dtime = (double)ret.time.tv_sec + (double)ret.time.tv_nsec / ONE_BILLION;
    ret.ips  = (double)ret.count / dtime;

    if (verbose)
        print_result(&ret, desc);

    return ret;
}

static void showUsage() {
    fprintf(stderr,
"Usage: %s [params] -n <count> | -s <bytes>\n"
"\n"
"    -v, --verbose\n"
"        Output verbose information to standard error.\n"
"\n"
"    -t, --max-time <time>\n"
"        Specify maxiumum time in seconds to run tests (floating point \n"
"        allowed). This only counts the time that actual sorting is\n"
"        running -- not setup time.\n"
"\n"
"    -i, --max-iterations <count>\n"
"        Maxiumum number of iterations to run for each test.\n"
"\n"
"    -n, --elem-count <count>\n"
"        Number of elements to use in array.\n"
"\n"
"    -s, --data-size <num_bytes>\n"
"        Size in bytes to use for array.\n"
"\n"
"    -h, --help\n"
"        Show this message, duh.\n",
            argv0);
}

static __attribute__((noreturn)) void
badOption(const struct option *opt, const char *str, const char *reason, int err) {
    fprintf(stderr, "ERROR: while parsing option --%s (-%c): %s, got %s.\n",
            opt->name, opt->val, reason, str);
    if (err)
        fprintf(stderr, "errno = %d\n", err);

    putc('\n', stderr);

    showUsage();

    exit(1);
}

static struct timespec parse_seconds(const struct option *opt, const char *str) {
    struct timespec ret;
    char *endptr = NULL;
    double dtime = strtod(optarg, &endptr);

    if (endptr == optarg)
        badOption(opt, str, "bad number (expected floating point number)", errno);

    ret.tv_sec = (time_t)dtime;
    dtime -= ret.tv_sec;
    ret.tv_nsec = (long)(dtime * ONE_BILLION);

    return ret;
}

static size_t parse_size_t(const struct option *opt, const char *str) {
    size_t ret;
    char *endptr;

    if (sizeof(size_t) <= sizeof(unsigned long))
        ret = strtoul(str, &endptr, 10);
    else
        ret = strtoull(str, &endptr, 10);

    if (str == endptr)
        badOption(opt, str, "bad number (expected size_t)", errno);

    return ret;
}

enum TEST_TYPES {
    TEST_QSORT,
    TEST_MSORT,
    TEST_TQSORT,
    TEST_COUNT
};

int main(int argc, char **argv) {
    void *arr;
    struct test_result results[TEST_COUNT];
    static const char *short_options = "vt:i:n:s:h?";
    static const struct option long_options[] = {
        /* These options set a flag. */
        {"verbose",         no_argument,        &verbose, 'v'},
        {"max-time",        required_argument,  NULL,     't'},
        {"max-iterations",  required_argument,  NULL,     'i'},
        {"elem-count",      required_argument,  NULL,     'n'},
        {"data-size",       required_argument,  NULL,     's'},
        {"help",            no_argument,        NULL,     'h'},
        {NULL, 0, NULL, 0}
    };
    int optind = 0;
    int c;
	int i;

    argv0 = *argv;

    /* verify that we have forced the object to whatever size we've
     * specified, even if it's stupid */
    gboing_assert(sizeof(struct size_type) == ELEM_SIZE);

    while ((c = getopt_long(argc, argv, short_options, long_options,
                            &optind)) != -1) {
        switch (c) {
        case 'v':
            verbose = 1;
            break;

        case 't':
            max_time = parse_seconds(&long_options[optind], optarg);
            break;

        case 'i':
            max_iterations = parse_size_t(&long_options[optind], optarg);
            break;

        case 'n':
            elem_count = parse_size_t(&long_options[optind], optarg);
            break;

        case 's':
            data_size = parse_size_t(&long_options[optind], optarg);
            break;

        case 'h':
        case '?':
        default:
            putc('\n', stderr);
            showUsage();
            exit(1);
        }
    }

//	fprintf(stderr, "%lu %lu %ld", sizeof(max_align_t), _Alignof(long), __STDC_VERSION__);//, sizeof(max_align_t));
//	exit(1);

    if (!(max_time.tv_sec | max_time.tv_nsec | max_iterations)) {
        fprintf(stderr, "ERROR: must specify either time limit (--max-time) "
                        "of maxiumum number of test iterations "
                        "(--max-iterations)\n");
        showUsage();
        abort();
    }

    if (elem_count && data_size) {
        fprintf(stderr, "ERROR: specify either --elem-count or --data-size, "
                        "but not both.\n\n");
        showUsage();
        abort();
    }

    if (elem_count)
        data_size = (size_t)(ELEM_SIZE) * elem_count;
    else
        elem_count = data_size / (size_t)(ELEM_SIZE);

    if (elem_count == 0) {
        fprintf(stderr, "ERROR: --data-size (%lu) too small for elements of "
                        "%lu bytes\n\n", data_size, (size_t)(ELEM_SIZE));
        abort();
    }

    if (MAX_SIZE_BITS) {
        size_t max_count = (MAX_SIZE_BITS)
                         ? (((size_t)1 << (MAX_SIZE_BITS)) - 1)
                         : (size_t)-1;

        if (elem_count > max_count) {
            fprintf(stderr, "elem_count (%lu) exceeds the max_size_bits (%u) "
                            "of this build (max_count=%lu)\n\n", elem_count,
                    (MAX_SIZE_BITS), max_count);
            abort();
        }
    }

    if (verbose) {
        fprintf(stderr,
               "\n\nRunning tests with:\n"
               "max_time       = %lu.%09lu\n"
               "max_iterations = %lu\n"
               "n              = %lu\n"
               "data_size      = %lu\n"
               "elem_size      = %lu\n"
               "min_align      = %lu\n"
               "key_type       = %s%u_t\n"
               "less_fn        = %s\n"
               "outline_copy   = %u\n"
               "outline_swap   = %u\n"
               "supply_buffer  = %u\n"
               "max_size_bits  = %u\n"
               "max_thresh     = %u\n",
               max_time.tv_sec, max_time.tv_nsec,
               max_iterations,
               elem_count,
               data_size,
               (size_t)(ELEM_SIZE),
               (size_t)(ALIGN_SIZE),
               GBOING_STRIZE(KEY_SIGN), KEY_BITS,
               GBOING_STRIZE(LESS_FN),
               OUTLINE_COPY,
               OUTLINE_SWAP,
               SUPPLY_BUFFER,
               MAX_SIZE_BITS,
               MAX_THRESH
               );
    }

    validate_sort(elem_count, ELEM_SIZE, ALIGN_SIZE, 0);

    arr = aligned_alloc(ALIGN_SIZE, data_size);
    if (gboing_unlikely(!arr)) {
        errno = ENOMEM;
        fatal_error("malloc %lu bytes\n", data_size);
    }

    results[TEST_QSORT]  = run_test(arr, 0, _quicksort, "_quicksort");
    results[TEST_MSORT]  = run_test(arr, 0, qsort_r, "qsort_r");
    results[TEST_TQSORT] = run_test(arr, 0, my_quicksort, "my_quicksort");

    if (verbose) {
        fprintf(stderr, "%.2f%% faster than _quicksort\n", results[TEST_TQSORT].ips / results[TEST_QSORT].ips * 100);
        fprintf(stderr, "%.2f%% faster than msort\n", results[TEST_TQSORT].ips / results[TEST_MSORT].ips * 100);
    }

    for (i = 0; i < TEST_COUNT; ++i) {
        if (i)
            fputs(" ", stdout);

        printf("%lu %lu.%09lu", results[i].count, results[i].time.tv_sec,
                results[i].time.tv_nsec);
    }
    putchar('\n');

    free (arr);
    return 0;
}


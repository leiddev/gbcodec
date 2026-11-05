/*
 * Minimal assertion helpers shared by the unit tests.
 *
 * Copyright (c) 2016-2026 leid
 * SPDX-License-Identifier: ISC
 */
#ifndef GBCODEC_TEST_COMMON_H
#define GBCODEC_TEST_COMMON_H

#include <stddef.h>
#include <stdio.h>
#include <string.h>

static int gbc_test_checks = 0;
static int gbc_test_failures = 0;

static inline int gbc_test_bytes_equal(const unsigned char *actual,
                                       const unsigned char *expected, size_t count)
{
    size_t index;

    for (index = 0; index < count; ++index) {
        if (actual[index] != expected[index]) {
            return 0;
        }
    }
    return 1;
}

#define CHECK(condition)                                                       \
    do {                                                                       \
        ++gbc_test_checks;                                                     \
        if (!(condition)) {                                                    \
            ++gbc_test_failures;                                               \
            fprintf(stderr, "%s:%d: CHECK failed: %s\n", __FILE__, __LINE__,   \
                    #condition);                                               \
        }                                                                      \
    } while (0)

#define CHECK_EQ(actual, expected)                                             \
    do {                                                                       \
        long long gbc_actual_ = (long long)(actual);                           \
        long long gbc_expected_ = (long long)(expected);                       \
        ++gbc_test_checks;                                                     \
        if (gbc_actual_ != gbc_expected_) {                                    \
            ++gbc_test_failures;                                               \
            fprintf(stderr,                                                    \
                    "%s:%d: CHECK_EQ failed: %s = %lld, expected %s = %lld\n", \
                    __FILE__, __LINE__, #actual, gbc_actual_, #expected,       \
                    gbc_expected_);                                            \
        }                                                                      \
    } while (0)

#define CHECK_BYTES(actual, expected, count)                                   \
    do {                                                                       \
        ++gbc_test_checks;                                                     \
        if (!gbc_test_bytes_equal((const unsigned char *)(actual),             \
                                  (const unsigned char *)(expected),           \
                                  (size_t)(count))) {                          \
            ++gbc_test_failures;                                               \
            fprintf(stderr, "%s:%d: CHECK_BYTES failed: %s\n", __FILE__,       \
                    __LINE__, #actual);                                        \
        }                                                                      \
    } while (0)

#define TEST_REPORT()                                                          \
    do {                                                                       \
        printf("%s: %d checks, %d failures\n", __FILE__, gbc_test_checks,      \
               gbc_test_failures);                                             \
        return gbc_test_failures == 0 ? 0 : 1;                                 \
    } while (0)

#endif /* GBCODEC_TEST_COMMON_H */

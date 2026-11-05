/*
 * gbcodec - minimal test harness shared by the C test programs.
 *
 * Copyright (c) 2016-2026 leid
 * SPDX-License-Identifier: ISC
 *
 * Every test program is a stand-alone executable: it includes this header,
 * calls the CHECK macros, and returns gbc_test_result() from main().
 */
#ifndef GBCODEC_TEST_COMMON_H
#define GBCODEC_TEST_COMMON_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

static int gbc_test_checks = 0;
static int gbc_test_failures = 0;

static inline int gbc_test_bytes_equal(const uint8_t *actual, const uint8_t *expected,
                                       size_t length)
{
    size_t index;

    for (index = 0u; index < length; ++index) {
        if (actual[index] != expected[index]) {
            return 0;
        }
    }
    return 1;
}

#define CHECK(condition)                                                                     \
    do {                                                                                     \
        ++gbc_test_checks;                                                                   \
        if (!(condition)) {                                                                  \
            ++gbc_test_failures;                                                             \
            fprintf(stderr, "%s:%d: CHECK(%s) failed\n", __FILE__, __LINE__, #condition);    \
        }                                                                                    \
    } while (0)

#define CHECK_EQ(actual, expected)                                                           \
    do {                                                                                     \
        const long long gbc_actual_value_ = (long long)(actual);                             \
        const long long gbc_expected_value_ = (long long)(expected);                         \
        ++gbc_test_checks;                                                                   \
        if (gbc_actual_value_ != gbc_expected_value_) {                                      \
            ++gbc_test_failures;                                                             \
            fprintf(stderr, "%s:%d: CHECK_EQ(%s, %s) failed: %lld != %lld\n", __FILE__,      \
                    __LINE__, #actual, #expected, gbc_actual_value_, gbc_expected_value_);   \
        }                                                                                    \
    } while (0)

#define CHECK_BYTES(actual, expected, length)                                                \
    do {                                                                                     \
        const uint8_t *gbc_actual_bytes_ = (const uint8_t *)(actual);                        \
        const uint8_t *gbc_expected_bytes_ = (const uint8_t *)(expected);                    \
        const size_t gbc_length_ = (size_t)(length);                                         \
        ++gbc_test_checks;                                                                   \
        if (!gbc_test_bytes_equal(gbc_actual_bytes_, gbc_expected_bytes_, gbc_length_)) {    \
            size_t gbc_index_;                                                               \
            ++gbc_test_failures;                                                             \
            fprintf(stderr, "%s:%d: CHECK_BYTES(%s, %s) failed\n", __FILE__, __LINE__,       \
                    #actual, #expected);                                                     \
            fprintf(stderr, "  actual:  ");                                                  \
            for (gbc_index_ = 0u; gbc_index_ < gbc_length_; ++gbc_index_) {                  \
                fprintf(stderr, "%02X ", (unsigned)gbc_actual_bytes_[gbc_index_]);           \
            }                                                                                \
            fprintf(stderr, "\n  expected:");                                                \
            for (gbc_index_ = 0u; gbc_index_ < gbc_length_; ++gbc_index_) {                  \
                fprintf(stderr, " %02X", (unsigned)gbc_expected_bytes_[gbc_index_]);         \
            }                                                                                \
            fprintf(stderr, "\n");                                                           \
        }                                                                                    \
    } while (0)

/* Prints a one-line summary and returns the process exit code. */
static inline int gbc_test_result(void)
{
    printf("%d checks, %d failures\n", gbc_test_checks, gbc_test_failures);
    return gbc_test_failures == 0 ? 0 : 1;
}

#endif /* GBCODEC_TEST_COMMON_H */

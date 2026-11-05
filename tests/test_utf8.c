/*
 * gbcodec - tests for the UTF-8 codec.
 *
 * Copyright (c) 2016-2026 leid
 * SPDX-License-Identifier: ISC
 */
#include <gbcodec/utf8.h>

#include <stddef.h>
#include <stdint.h>

#include "test_common.h"

static void test_ascii(void)
{
    static const uint8_t bytes[] = {'A'};
    uint32_t code_point = 0u;
    size_t consumed = 0u;
    uint8_t encoded[4] = {0u};
    size_t produced;

    CHECK_EQ(gbc_utf8_decode(bytes, sizeof(bytes), &code_point, &consumed), GBC_OK);
    CHECK_EQ(code_point, 0x41u);
    CHECK_EQ(consumed, 1u);

    produced = gbc_utf8_encode(0x41u, encoded);
    CHECK_EQ(produced, 1u);
    CHECK_BYTES(encoded, bytes, 1u);
}

static void test_two_byte(void)
{
    static const uint8_t bytes[] = {0xC2u, 0xA2u};
    uint32_t code_point = 0u;
    size_t consumed = 0u;
    uint8_t encoded[4] = {0u};

    CHECK_EQ(gbc_utf8_decode(bytes, sizeof(bytes), &code_point, &consumed), GBC_OK);
    CHECK_EQ(code_point, 0xA2u);
    CHECK_EQ(consumed, 2u);
    CHECK_EQ(gbc_utf8_encode(0xA2u, encoded), 2u);
    CHECK_BYTES(encoded, bytes, 2u);
}

static void test_three_byte(void)
{
    static const uint8_t bytes[] = {0xE4u, 0xB8u, 0xADu};
    uint32_t code_point = 0u;
    size_t consumed = 0u;
    uint8_t encoded[4] = {0u};

    /* U+4E2D CJK UNIFIED IDEOGRAPH-4E2D. */
    CHECK_EQ(gbc_utf8_decode(bytes, sizeof(bytes), &code_point, &consumed), GBC_OK);
    CHECK_EQ(code_point, 0x4E2Du);
    CHECK_EQ(consumed, 3u);
    CHECK_EQ(gbc_utf8_encode(0x4E2Du, encoded), 3u);
    CHECK_BYTES(encoded, bytes, 3u);
}

static void test_four_byte(void)
{
    static const uint8_t bytes[] = {0xF0u, 0x9Fu, 0x98u, 0x80u};
    uint32_t code_point = 0u;
    size_t consumed = 0u;
    uint8_t encoded[4] = {0u};

    /* U+1F600 GRINNING FACE. */
    CHECK_EQ(gbc_utf8_decode(bytes, sizeof(bytes), &code_point, &consumed), GBC_OK);
    CHECK_EQ(code_point, 0x1F600u);
    CHECK_EQ(consumed, 4u);
    CHECK_EQ(gbc_utf8_encode(0x1F600u, encoded), 4u);
    CHECK_BYTES(encoded, bytes, 4u);
}

static void test_boundaries(void)
{
    static const uint32_t code_points[] = {0x00u, 0x7Fu, 0x80u, 0x7FFu, 0x800u,
                                           0xFFFu, 0xFFFFu, 0x10000u, 0x10FFFFu};
    size_t index;

    for (index = 0u; index < sizeof(code_points) / sizeof(code_points[0]); ++index) {
        uint8_t encoded[4] = {0u};
        uint32_t decoded = 0u;
        size_t consumed = 0u;
        const size_t produced = gbc_utf8_encode(code_points[index], encoded);

        CHECK(produced >= 1u && produced <= 4u);
        CHECK_EQ(gbc_utf8_decode(encoded, produced, &decoded, &consumed), GBC_OK);
        CHECK_EQ(decoded, code_points[index]);
        CHECK_EQ(consumed, produced);
    }
}

static void test_invalid_sequences(void)
{
    static const uint8_t stray_continuation[] = {0x80u};
    static const uint8_t overlong_two[] = {0xC0u, 0x80u};
    static const uint8_t overlong_three[] = {0xE0u, 0x80u, 0x80u};
    static const uint8_t overlong_four[] = {0xF0u, 0x80u, 0x80u, 0x80u};
    static const uint8_t surrogate[] = {0xEDu, 0xA0u, 0x80u};
    static const uint8_t above_maximum[] = {0xF4u, 0x90u, 0x80u, 0x80u};
    static const uint8_t invalid_lead[] = {0xF5u, 0x80u, 0x80u, 0x80u};
    static const uint8_t all_ones[] = {0xFFu};
    static const uint8_t bad_continuation[] = {0xE4u, 0x00u, 0x00u};
    const struct {
        const uint8_t *bytes;
        size_t length;
    } cases[] = {{stray_continuation, sizeof(stray_continuation)},
                 {overlong_two, sizeof(overlong_two)},
                 {overlong_three, sizeof(overlong_three)},
                 {overlong_four, sizeof(overlong_four)},
                 {surrogate, sizeof(surrogate)},
                 {above_maximum, sizeof(above_maximum)},
                 {invalid_lead, sizeof(invalid_lead)},
                 {all_ones, sizeof(all_ones)},
                 {bad_continuation, sizeof(bad_continuation)}};
    size_t index;

    for (index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        uint32_t code_point = 0xDEADBEEFu;
        size_t consumed = 99u;

        CHECK_EQ(gbc_utf8_decode(cases[index].bytes, cases[index].length, &code_point, &consumed),
                 GBC_E_INVALID_SEQUENCE);
        /* Outputs must be untouched on failure. */
        CHECK_EQ(code_point, 0xDEADBEEFu);
        CHECK_EQ(consumed, 99u);
    }
}

static void test_truncated(void)
{
    static const uint8_t one_byte[] = {0xE4u};
    static const uint8_t two_bytes[] = {0xE4u, 0xB8u};
    static const uint8_t three_bytes[] = {0xF0u, 0x9Fu, 0x98u};
    const struct {
        const uint8_t *bytes;
        size_t length;
    } cases[] = {{one_byte, sizeof(one_byte)},
                 {two_bytes, sizeof(two_bytes)},
                 {three_bytes, sizeof(three_bytes)},
                 {one_byte, 0u}};
    size_t index;

    for (index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        uint32_t code_point = 0u;
        size_t consumed = 0u;

        CHECK_EQ(gbc_utf8_decode(cases[index].bytes, cases[index].length, &code_point, &consumed),
                 GBC_E_TRUNCATED);
    }
}

static void test_invalid_arguments(void)
{
    static const uint8_t bytes[] = {'A'};
    uint32_t code_point = 0u;
    size_t consumed = 0u;

    CHECK_EQ(gbc_utf8_decode(NULL, 1u, &code_point, &consumed), GBC_E_INVALID_ARGUMENT);
    CHECK_EQ(gbc_utf8_decode(bytes, 1u, NULL, &consumed), GBC_E_INVALID_ARGUMENT);
    CHECK_EQ(gbc_utf8_decode(bytes, 1u, &code_point, NULL), GBC_E_INVALID_ARGUMENT);
    CHECK_EQ(gbc_utf8_encode(0x41u, NULL), 0u);
}

static void test_encode_rejects(void)
{
    uint8_t encoded[4] = {0u};

    CHECK_EQ(gbc_utf8_encode(0xD800u, encoded), 0u);
    CHECK_EQ(gbc_utf8_encode(0xDC00u, encoded), 0u);
    CHECK_EQ(gbc_utf8_encode(0x110000u, encoded), 0u);
    CHECK_EQ(gbc_utf8_encode(0xFFFFFFFEu, encoded), 0u);
}

int main(void)
{
    test_ascii();
    test_two_byte();
    test_three_byte();
    test_four_byte();
    test_boundaries();
    test_invalid_sequences();
    test_truncated();
    test_invalid_arguments();
    test_encode_rejects();
    return gbc_test_result();
}

/*
 * gbcodec - tests for the UTF-16 codec.
 *
 * Copyright (c) 2016-2026 leid
 * SPDX-License-Identifier: ISC
 */
#include <gbcodec/utf16.h>

#include <stddef.h>
#include <stdint.h>

#include "test_common.h"

static void test_basic_plane(void)
{
    static const uint16_t ascii[] = {0x0041u};
    static const uint16_t hanzi[] = {0x4E2Du};
    uint32_t code_point = 0u;
    size_t consumed = 0u;
    uint16_t encoded[2] = {0u, 0u};

    CHECK_EQ(gbc_utf16_decode(ascii, 1u, &code_point, &consumed), GBC_OK);
    CHECK_EQ(code_point, 0x41u);
    CHECK_EQ(consumed, 1u);

    CHECK_EQ(gbc_utf16_decode(hanzi, 1u, &code_point, &consumed), GBC_OK);
    CHECK_EQ(code_point, 0x4E2Du);
    CHECK_EQ(consumed, 1u);

    CHECK_EQ(gbc_utf16_encode(0x4E2Du, encoded), 1u);
    CHECK_EQ(encoded[0], 0x4E2Du);
}

static void test_surrogate_pair(void)
{
    static const uint16_t pair[] = {0xD83Du, 0xDE00u};
    uint32_t code_point = 0u;
    size_t consumed = 0u;
    uint16_t encoded[2] = {0u, 0u};

    /* U+1F600 encodes as D83D DE00. */
    CHECK_EQ(gbc_utf16_decode(pair, 2u, &code_point, &consumed), GBC_OK);
    CHECK_EQ(code_point, 0x1F600u);
    CHECK_EQ(consumed, 2u);

    CHECK_EQ(gbc_utf16_encode(0x1F600u, encoded), 2u);
    CHECK_EQ(encoded[0], 0xD83Du);
    CHECK_EQ(encoded[1], 0xDE00u);
}

static void test_boundaries(void)
{
    static const uint32_t code_points[] = {0x0000u, 0x7FFFu, 0x8000u, 0xD7FFu,
                                           0xE000u, 0xFFFFu, 0x10000u, 0x10FFFFu};
    size_t index;

    for (index = 0u; index < sizeof(code_points) / sizeof(code_points[0]); ++index) {
        uint16_t encoded[2] = {0u, 0u};
        uint32_t decoded = 0u;
        size_t consumed = 0u;
        const size_t produced = gbc_utf16_encode(code_points[index], encoded);

        CHECK(produced == 1u || produced == 2u);
        CHECK_EQ(gbc_utf16_decode(encoded, produced, &decoded, &consumed), GBC_OK);
        CHECK_EQ(decoded, code_points[index]);
        CHECK_EQ(consumed, produced);
    }
}

static void test_invalid_sequences(void)
{
    static const uint16_t lone_low[] = {0xDE00u};
    static const uint16_t high_then_high[] = {0xD83Du, 0xD83Du};
    static const uint16_t high_then_ascii[] = {0xD83Du, 0x0041u};
    static const uint16_t lone_high[] = {0xD83Du};
    uint32_t code_point = 0xDEADBEEFu;
    size_t consumed = 99u;

    CHECK_EQ(gbc_utf16_decode(lone_low, 1u, &code_point, &consumed), GBC_E_INVALID_SEQUENCE);
    CHECK_EQ(gbc_utf16_decode(high_then_high, 2u, &code_point, &consumed),
             GBC_E_INVALID_SEQUENCE);
    CHECK_EQ(gbc_utf16_decode(high_then_ascii, 2u, &code_point, &consumed),
             GBC_E_INVALID_SEQUENCE);
    CHECK_EQ(gbc_utf16_decode(lone_high, 1u, &code_point, &consumed), GBC_E_TRUNCATED);
    CHECK_EQ(code_point, 0xDEADBEEFu);
    CHECK_EQ(consumed, 99u);
}

static void test_invalid_arguments(void)
{
    static const uint16_t units[] = {0x0041u};
    uint32_t code_point = 0u;
    size_t consumed = 0u;

    CHECK_EQ(gbc_utf16_decode(NULL, 1u, &code_point, &consumed), GBC_E_INVALID_ARGUMENT);
    CHECK_EQ(gbc_utf16_decode(units, 0u, &code_point, &consumed), GBC_E_TRUNCATED);
    CHECK_EQ(gbc_utf16_decode(units, 1u, NULL, &consumed), GBC_E_INVALID_ARGUMENT);
    CHECK_EQ(gbc_utf16_decode(units, 1u, &code_point, NULL), GBC_E_INVALID_ARGUMENT);
    CHECK_EQ(gbc_utf16_encode(0x41u, NULL), 0u);
}

static void test_encode_rejects(void)
{
    uint16_t encoded[2] = {0u, 0u};

    CHECK_EQ(gbc_utf16_encode(0xD800u, encoded), 0u);
    CHECK_EQ(gbc_utf16_encode(0xDFFFu, encoded), 0u);
    CHECK_EQ(gbc_utf16_encode(0x110000u, encoded), 0u);
}

int main(void)
{
    test_basic_plane();
    test_surrogate_pair();
    test_boundaries();
    test_invalid_sequences();
    test_invalid_arguments();
    test_encode_rejects();
    return gbc_test_result();
}

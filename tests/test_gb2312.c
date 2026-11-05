/*
 * gbcodec - tests for the GB2312 codec.
 *
 * Copyright (c) 2016-2026 leid
 * SPDX-License-Identifier: ISC
 */
#include <gbcodec/gb2312.h>

#include <stddef.h>
#include <stdint.h>

#include "test_common.h"

/* Number of assigned characters in GB2312. */
#define GBC_TEST_EXPECTED_ENTRIES 7445u

static void test_ascii(void)
{
    static const uint8_t bytes[] = {'A'};
    uint32_t code_point = 0u;
    size_t consumed = 0u;
    uint8_t encoded[2] = {0u, 0u};

    CHECK_EQ(gbc_gb2312_decode(bytes, 1u, &code_point, &consumed), GBC_OK);
    CHECK_EQ(code_point, 0x41u);
    CHECK_EQ(consumed, 1u);

    CHECK_EQ(gbc_gb2312_encode(0x20u, encoded), 1u);
    CHECK_EQ(encoded[0], 0x20u);
}

static void test_known_mappings(void)
{
    static const uint8_t hanzi[] = {0xB0u, 0xA1u};
    static const uint8_t space[] = {0xA1u, 0xA1u};
    uint32_t code_point = 0u;
    size_t consumed = 0u;
    uint8_t encoded[2] = {0u, 0u};

    /* B0A1 <-> U+554A (啊). */
    CHECK_EQ(gbc_gb2312_decode(hanzi, 2u, &code_point, &consumed), GBC_OK);
    CHECK_EQ(code_point, 0x554Au);
    CHECK_EQ(consumed, 2u);
    CHECK_EQ(gbc_gb2312_encode(0x554Au, encoded), 2u);
    CHECK_BYTES(encoded, hanzi, 2u);

    /* A1A1 <-> U+3000 (ideographic space). */
    CHECK_EQ(gbc_gb2312_decode(space, 2u, &code_point, &consumed), GBC_OK);
    CHECK_EQ(code_point, 0x3000u);
    CHECK_EQ(gbc_gb2312_encode(0x3000u, encoded), 2u);
    CHECK_BYTES(encoded, space, 2u);
}

static void test_latin_one_regression(void)
{
    /* U+00A4 used to be looked up with a wrong signed comparison. */
    static const uint8_t expected[] = {0xA1u, 0xE8u};
    uint8_t encoded[2] = {0u, 0u};
    uint32_t code_point = 0u;
    size_t consumed = 0u;

    CHECK_EQ(gbc_gb2312_encode(0xA4u, encoded), 2u);
    CHECK_BYTES(encoded, expected, 2u);

    CHECK_EQ(gbc_gb2312_decode(expected, 2u, &code_point, &consumed), GBC_OK);
    CHECK_EQ(code_point, 0xA4u);
}

static void test_unmapped_input(void)
{
    /* AA A1 is a valid GB2312 byte pair but no character is assigned to it. */
    static const uint8_t unassigned[] = {0xAAu, 0xA1u};
    uint32_t code_point = 0xDEADBEEFu;
    size_t consumed = 99u;

    CHECK_EQ(gbc_gb2312_decode(unassigned, 2u, &code_point, &consumed), GBC_E_UNMAPPABLE);
    CHECK_EQ(code_point, 0xDEADBEEFu);
    CHECK_EQ(consumed, 99u);
}

static void test_unmappable_code_points(void)
{
    static const uint32_t code_points[] = {
        0x20ACu, /* EURO SIGN: GBK only. */
        0x2170u, /* SMALL ROMAN NUMERAL ONE: GBK only. */
        0xE000u, /* Private use area. */
        0x10000u /* Outside the basic multilingual plane. */
    };
    size_t index;

    for (index = 0u; index < sizeof(code_points) / sizeof(code_points[0]); ++index) {
        uint8_t encoded[2] = {0u, 0u};

        CHECK_EQ(gbc_gb2312_encode(code_points[index], encoded), 0u);
    }
}

static void test_invalid_sequences(void)
{
    static const uint8_t invalid_lead[] = {0x81u};
    static const uint8_t gbk_only_lead[] = {0xF8u, 0xA1u};
    static const uint8_t invalid_trail_low[] = {0xA1u, 0x40u};
    static const uint8_t invalid_trail_high[] = {0xA1u, 0xFFu};
    const struct {
        const uint8_t *bytes;
        size_t length;
    } cases[] = {{invalid_lead, 1u},
                 {gbk_only_lead, 2u},
                 {invalid_trail_low, 2u},
                 {invalid_trail_high, 2u}};
    size_t index;

    for (index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        uint32_t code_point = 0xDEADBEEFu;
        size_t consumed = 99u;

        CHECK_EQ(gbc_gb2312_decode(cases[index].bytes, cases[index].length, &code_point,
                                   &consumed),
                 GBC_E_INVALID_SEQUENCE);
        CHECK_EQ(code_point, 0xDEADBEEFu);
        CHECK_EQ(consumed, 99u);
    }
}

static void test_truncated_and_arguments(void)
{
    static const uint8_t lead_only[] = {0xA1u};
    static const uint8_t pair[] = {0xB0u, 0xA1u};
    uint32_t code_point = 0u;
    size_t consumed = 0u;
    uint8_t encoded[2] = {0u, 0u};

    CHECK_EQ(gbc_gb2312_decode(lead_only, 1u, &code_point, &consumed), GBC_E_TRUNCATED);
    CHECK_EQ(gbc_gb2312_decode(pair, 0u, &code_point, &consumed), GBC_E_TRUNCATED);
    CHECK_EQ(gbc_gb2312_decode(NULL, 1u, &code_point, &consumed), GBC_E_INVALID_ARGUMENT);
    CHECK_EQ(gbc_gb2312_decode(pair, 2u, NULL, &consumed), GBC_E_INVALID_ARGUMENT);
    CHECK_EQ(gbc_gb2312_decode(pair, 2u, &code_point, NULL), GBC_E_INVALID_ARGUMENT);
    CHECK_EQ(gbc_gb2312_encode(0x4E2Du, NULL), 0u);
    CHECK_EQ(gbc_gb2312_encode(0xD800u, encoded), 0u);
}

static void test_byte_range_is_exhaustive(void)
{
    uint32_t lead;
    size_t mapped = 0u;
    size_t unmapped = 0u;

    for (lead = 0xA1u; lead <= 0xF7u; ++lead) {
        uint32_t trail;

        for (trail = 0xA1u; trail <= 0xFEu; ++trail) {
            const uint8_t bytes[2] = {(uint8_t)lead, (uint8_t)trail};
            uint32_t code_point = 0u;
            size_t consumed = 0u;
            uint8_t encoded[2] = {0u, 0u};
            const gbc_status status = gbc_gb2312_decode(bytes, 2u, &code_point, &consumed);

            if (status == GBC_OK) {
                ++mapped;
                CHECK_EQ(consumed, 2u);
                CHECK_EQ(gbc_gb2312_encode(code_point, encoded), 2u);
                CHECK_BYTES(encoded, bytes, 2u);
            } else {
                CHECK_EQ(status, GBC_E_UNMAPPABLE);
                ++unmapped;
            }
        }
    }

    CHECK_EQ(mapped, GBC_TEST_EXPECTED_ENTRIES);
    CHECK_EQ(mapped + unmapped, 87u * 94u);
}

int main(void)
{
    test_ascii();
    test_known_mappings();
    test_latin_one_regression();
    test_unmapped_input();
    test_unmappable_code_points();
    test_invalid_sequences();
    test_truncated_and_arguments();
    test_byte_range_is_exhaustive();
    return gbc_test_result();
}

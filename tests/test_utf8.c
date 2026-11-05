/*
 * Tests for the UTF-8 codec.
 *
 * Copyright (c) 2016-2026 leid
 * SPDX-License-Identifier: ISC
 */
#include "test_common.h"
#include "unicode_utility.h"

static void check_decode(const unsigned char *input, uint32_t expected,
                         int32_t expected_length)
{
    uint8_t buffer[4] = {0, 0, 0, 0};
    uint32_t code_point = 0xFFFFFFFFu;
    int32_t consumed;

    memcpy(buffer, input, (size_t)expected_length);
    consumed = UTF8ToUnicode(buffer, &code_point);
    CHECK_EQ(consumed, expected_length);
    CHECK_EQ(code_point, expected);
}

static void check_decode_invalid(const unsigned char *input, size_t length)
{
    uint8_t buffer[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    uint32_t code_point = 0xFFFFFFFFu;

    memcpy(buffer, input, length);
    CHECK_EQ(UTF8ToUnicode(buffer, &code_point), -1);
    CHECK_EQ(code_point, 0xFFFFFFFFu);
}

static void test_decode_valid(void)
{
    static const unsigned char ascii[] = {0x41, 0x00};
    static const unsigned char two_byte[] = {0xC2, 0xA9};       /* U+00A9 */
    static const unsigned char three_byte[] = {0xE4, 0xB8, 0xAD}; /* U+4E2D */
    static const unsigned char four_byte[] = {0xF0, 0x9F, 0x98, 0x80}; /* U+1F600 */
    static const unsigned char max_cp[] = {0xF4, 0x8F, 0xBF, 0xBF};    /* U+10FFFF */
    static const unsigned char boundary[] = {0xF0, 0x90, 0x80, 0x80};  /* U+10000 */

    check_decode(ascii, 0x41u, 1);
    check_decode(two_byte, 0x00A9u, 2);
    check_decode(three_byte, 0x4E2Du, 3);
    check_decode(four_byte, 0x1F600u, 4);
    check_decode(max_cp, 0x10FFFFu, 4);
    check_decode(boundary, 0x10000u, 4);
}

static void test_decode_invalid(void)
{
    static const unsigned char lone_continuation[] = {0x80};
    static const unsigned char overlong_two[] = {0xC0, 0xAF};
    static const unsigned char overlong_two_c1[] = {0xC1, 0xBF};
    static const unsigned char overlong_three[] = {0xE0, 0x80, 0x80};
    static const unsigned char overlong_four[] = {0xF0, 0x80, 0x80, 0x80};
    static const unsigned char surrogate[] = {0xED, 0xA0, 0x80}; /* U+D800 */
    static const unsigned char too_large[] = {0xF4, 0x90, 0x80, 0x80}; /* U+110000 */
    static const unsigned char bad_lead_f5[] = {0xF5, 0x80, 0x80, 0x80};
    static const unsigned char bad_lead_fe[] = {0xFE};
    static const unsigned char bad_lead_ff[] = {0xFF};
    static const unsigned char truncated_two[] = {0xE4, 0xB8, 0x00};
    static const unsigned char truncated_four[] = {0xF0, 0x9F, 0x00, 0x00};

    check_decode_invalid(lone_continuation, sizeof lone_continuation);
    check_decode_invalid(overlong_two, sizeof overlong_two);
    check_decode_invalid(overlong_two_c1, sizeof overlong_two_c1);
    check_decode_invalid(overlong_three, sizeof overlong_three);
    check_decode_invalid(overlong_four, sizeof overlong_four);
    check_decode_invalid(surrogate, sizeof surrogate);
    check_decode_invalid(too_large, sizeof too_large);
    check_decode_invalid(bad_lead_f5, sizeof bad_lead_f5);
    check_decode_invalid(bad_lead_fe, sizeof bad_lead_fe);
    check_decode_invalid(bad_lead_ff, sizeof bad_lead_ff);
    check_decode_invalid(truncated_two, sizeof truncated_two);
    check_decode_invalid(truncated_four, sizeof truncated_four);
}

static void test_decode_null(void)
{
    uint32_t code_point = 0;
    uint8_t buffer[4] = {0x41, 0, 0, 0};

    CHECK_EQ(UTF8ToUnicode(NULL, &code_point), -1);
    CHECK_EQ(UTF8ToUnicode(buffer, NULL), -1);
}

static void test_encode(void)
{
    static const struct {
        uint32_t code_point;
        const unsigned char *bytes;
        int32_t length;
    } cases[] = {
        {0x0000u, (const unsigned char *)"\x00", 1},
        {0x0041u, (const unsigned char *)"\x41", 1},
        {0x007Fu, (const unsigned char *)"\x7F", 1},
        {0x0080u, (const unsigned char *)"\xC2\x80", 2},
        {0x07FFu, (const unsigned char *)"\xDF\xBF", 2},
        {0x0800u, (const unsigned char *)"\xE0\xA0\x80", 3},
        {0x4E2Du, (const unsigned char *)"\xE4\xB8\xAD", 3},
        {0xFFFFu, (const unsigned char *)"\xEF\xBF\xBF", 3},
        {0x10000u, (const unsigned char *)"\xF0\x90\x80\x80", 4},
        {0x10FFFFu, (const unsigned char *)"\xF4\x8F\xBF\xBF", 4},
    };
    size_t index;

    for (index = 0; index < sizeof cases / sizeof cases[0]; ++index) {
        uint8_t buffer[4] = {0, 0, 0, 0};
        int32_t length = UnicodeToUTF8(cases[index].code_point, buffer);

        CHECK_EQ(length, cases[index].length);
        CHECK_BYTES(buffer, cases[index].bytes, (size_t)cases[index].length);
    }
}

static void test_encode_invalid(void)
{
    uint8_t buffer[4] = {0, 0, 0, 0};

    CHECK_EQ(UnicodeToUTF8(0xD800u, buffer), 0);
    CHECK_EQ(UnicodeToUTF8(0xDFFFu, buffer), 0);
    CHECK_EQ(UnicodeToUTF8(0x110000u, buffer), 0);
    CHECK_EQ(UnicodeToUTF8(0xFFFFFFFFu, buffer), 0);
    CHECK_EQ(UnicodeToUTF8(0x41u, NULL), 0);
}

static void test_round_trip(void)
{
    uint32_t code_point;

    for (code_point = 0; code_point < 0x2000u; ++code_point) {
        uint8_t encoded[4];
        int32_t length;

        if (code_point >= 0xD800u && code_point <= 0xDFFFu) {
            continue;
        }
        length = UnicodeToUTF8(code_point, encoded);
        CHECK_EQ(length, (int32_t)(code_point < 0x80u ? 1 : (code_point < 0x800u ? 2 : 3)));
        {
            uint32_t decoded = 0;
            CHECK_EQ(UTF8ToUnicode(encoded, &decoded), length);
            CHECK_EQ(decoded, code_point);
        }
    }
}

int main(void)
{
    test_decode_valid();
    test_decode_invalid();
    test_decode_null();
    test_encode();
    test_encode_invalid();
    test_round_trip();
    TEST_REPORT();
}

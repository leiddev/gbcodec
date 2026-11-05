/*
 * Tests for the UTF-16 codec.
 *
 * Copyright (c) 2016-2026 leid
 * SPDX-License-Identifier: ISC
 */
#include "test_common.h"
#include "unicode_utility.h"

static void test_decode(void)
{
    uint16_t single[] = {0x0041, 0x0000};
    uint16_t bmp[] = {0x4E2D, 0x0000};
    uint16_t pair[] = {0xD83D, 0xDE00}; /* U+1F600 */
    uint16_t max_pair[] = {0xDBFF, 0xDFFF}; /* U+10FFFF */
    uint32_t code_point = 0;

    CHECK_EQ(UTF16toUnicode(single, &code_point), 1);
    CHECK_EQ(code_point, 0x0041u);

    CHECK_EQ(UTF16toUnicode(bmp, &code_point), 1);
    CHECK_EQ(code_point, 0x4E2Du);

    CHECK_EQ(UTF16toUnicode(pair, &code_point), 2);
    CHECK_EQ(code_point, 0x1F600u);

    CHECK_EQ(UTF16toUnicode(max_pair, &code_point), 2);
    CHECK_EQ(code_point, 0x10FFFFu);
}

static void test_decode_invalid(void)
{
    uint16_t lone_high[] = {0xD800, 0x0000};
    uint16_t lone_low[] = {0xDC00, 0x0000};
    uint16_t high_then_ascii[] = {0xD83D, 0x0041};
    uint16_t high_then_high[] = {0xD83D, 0xD83D};
    uint32_t code_point;

    code_point = 0xFFFFFFFFu;
    CHECK_EQ(UTF16toUnicode(lone_high, &code_point), -1);
    CHECK_EQ(code_point, 0xFFFFFFFFu);

    code_point = 0xFFFFFFFFu;
    CHECK_EQ(UTF16toUnicode(lone_low, &code_point), -1);

    code_point = 0xFFFFFFFFu;
    CHECK_EQ(UTF16toUnicode(high_then_ascii, &code_point), -1);

    code_point = 0xFFFFFFFFu;
    CHECK_EQ(UTF16toUnicode(high_then_high, &code_point), -1);

    code_point = 0;
    CHECK_EQ(UTF16toUnicode(NULL, &code_point), -1);
    CHECK_EQ(UTF16toUnicode(lone_high, NULL), -1);
}

static void test_encode(void)
{
    uint16_t buffer[2] = {0, 0};

    CHECK_EQ(UnicodeToUTF16(0x0041u, buffer), 1);
    CHECK_EQ(buffer[0], 0x0041);

    CHECK_EQ(UnicodeToUTF16(0x4E2Du, buffer), 1);
    CHECK_EQ(buffer[0], 0x4E2D);

    CHECK_EQ(UnicodeToUTF16(0x10000u, buffer), 2);
    CHECK_EQ(buffer[0], 0xD800);
    CHECK_EQ(buffer[1], 0xDC00);

    CHECK_EQ(UnicodeToUTF16(0x1F600u, buffer), 2);
    CHECK_EQ(buffer[0], 0xD83D);
    CHECK_EQ(buffer[1], 0xDE00);

    CHECK_EQ(UnicodeToUTF16(0x10FFFFu, buffer), 2);
    CHECK_EQ(buffer[0], 0xDBFF);
    CHECK_EQ(buffer[1], 0xDFFF);
}

static void test_encode_invalid(void)
{
    uint16_t buffer[2] = {0, 0};

    CHECK_EQ(UnicodeToUTF16(0xD800u, buffer), -1);
    CHECK_EQ(UnicodeToUTF16(0xDFFFu, buffer), -1);
    CHECK_EQ(UnicodeToUTF16(0x110000u, buffer), -1);
    CHECK_EQ(UnicodeToUTF16(0x41u, NULL), -1);
}

static void test_round_trip(void)
{
    static const uint32_t samples[] = {
        0x0000u, 0x0041u, 0x007Fu, 0x0080u, 0x07FFu, 0x0800u,
        0x4E2Du, 0xD7FFu, 0xE000u, 0xFFFFu, 0x10000u, 0x1F600u, 0x10FFFFu,
    };
    size_t index;

    for (index = 0; index < sizeof samples / sizeof samples[0]; ++index) {
        uint16_t stage[2] = {0, 0};
        uint32_t decoded = 0;
        int32_t units = UnicodeToUTF16(samples[index], stage);

        CHECK(units > 0);
        CHECK_EQ(UTF16toUnicode(stage, &decoded), units);
        CHECK_EQ(decoded, samples[index]);
    }
}

int main(void)
{
    test_decode();
    test_decode_invalid();
    test_encode();
    test_encode_invalid();
    test_round_trip();
    TEST_REPORT();
}

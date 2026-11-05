/*
 * Exhaustive round-trip tests across the whole GB2312 code space.
 *
 * Copyright (c) 2016-2026 leid
 * SPDX-License-Identifier: ISC
 */
#include "test_common.h"
#include "unicode_utility.h"

static void test_two_byte_space(void)
{
    unsigned int lead;
    unsigned int trail;
    int mapped = 0;

    for (lead = 0x81u; lead <= 0xFEu; ++lead) {
        for (trail = 0x00u; trail <= 0xFFu; ++trail) {
            uint8_t source[3];
            uint8_t utf8[8];
            uint8_t back[8];
            int32_t utf8_length;
            int32_t back_length;

            source[0] = (uint8_t)lead;
            source[1] = (uint8_t)trail;
            source[2] = 0x00;

            utf8_length = GB2312ToUTF8Str(source, utf8, sizeof utf8);
            if (utf8_length <= 0) {
                continue;
            }

            back_length = UTF8ToGB2312Str(utf8, back, sizeof back);
            CHECK_EQ(back_length, 2);
            CHECK_EQ(back[0], lead);
            CHECK_EQ(back[1], trail);
            ++mapped;
        }
    }

    printf("mapped two-byte codes: %d\n", mapped);
    CHECK(mapped >= 7400);
}

static void test_ascii_space(void)
{
    unsigned int value;

    for (value = 0x01u; value <= 0x7Fu; ++value) {
        uint8_t source[2];
        uint8_t gb2312[4];
        uint8_t utf8[4];
        int32_t gb_length;

        source[0] = (uint8_t)value;
        source[1] = 0x00;

        gb_length = UTF8ToGB2312Str(source, gb2312, sizeof gb2312);
        CHECK_EQ(gb_length, 1);
        CHECK_EQ(gb2312[0], value);

        CHECK_EQ(GB2312ToUTF8Str(gb2312, utf8, sizeof utf8), 1);
        CHECK_EQ(utf8[0], value);
    }
}

int main(void)
{
    test_two_byte_space();
    test_ascii_space();
    TEST_REPORT();
}

/*
 * Tests for GB2312 encoding, decoding and validation.
 *
 * Copyright (c) 2016-2026 leid
 * SPDX-License-Identifier: ISC
 */
#include "test_common.h"
#include "unicode_utility.h"

static void test_encode_ascii(void)
{
    uint16_t buffer = 0;
    const unsigned char *bytes = (const unsigned char *)&buffer;

    CHECK_EQ(UTF16ToGB2312(0x41u, &buffer), 1);
    CHECK_EQ(bytes[0], 0x41);
}

static void test_encode_cjk(void)
{
    uint16_t buffer = 0;
    const unsigned char *bytes = (const unsigned char *)&buffer;

    /* U+4E2D is 0xD6D0 in GB2312 and must be stored as D6 D0. */
    CHECK_EQ(UTF16ToGB2312(0x4E2Du, &buffer), 2);
    CHECK_EQ(bytes[0], 0xD6);
    CHECK_EQ(bytes[1], 0xD0);
}

static void test_latin_one_regression(void)
{
    /* U+00A4 used to be written as a single 0xA4 byte because of a wrong
     * "ASCII" threshold. It must map to the two-byte GB2312 code A1E8. */
    uint16_t buffer = 0;
    const unsigned char *bytes = (const unsigned char *)&buffer;

    CHECK_EQ(UTF16ToGB2312(0x00A4u, &buffer), 2);
    CHECK_EQ(bytes[0], 0xA1);
    CHECK_EQ(bytes[1], 0xE8);
}

static void test_encode_unmappable(void)
{
    uint16_t buffer = 0;

    CHECK_EQ(UTF16ToGB2312(0xD800u, &buffer), -1);   /* surrogate */
    CHECK_EQ(UTF16ToGB2312(0xFFFDu, &buffer), -1);   /* never mapped */
    CHECK_EQ(UTF16ToGB2312(0x0041u, NULL), -1);
}

static void test_decode(void)
{
    uint16_t buffer;
    uint16_t code_point = 0;

    buffer = 0;
    ((unsigned char *)&buffer)[0] = 0x41;
    CHECK_EQ(GB2312ToUTF16(buffer, &code_point), 1);
    CHECK_EQ(code_point, 0x0041);

    buffer = 0;
    ((unsigned char *)&buffer)[0] = 0xD6;
    ((unsigned char *)&buffer)[1] = 0xD0;
    CHECK_EQ(GB2312ToUTF16(buffer, &code_point), 2);
    CHECK_EQ(code_point, 0x4E2D);

    buffer = 0;
    ((unsigned char *)&buffer)[0] = 0xA1;
    ((unsigned char *)&buffer)[1] = 0xE8;
    CHECK_EQ(GB2312ToUTF16(buffer, &code_point), 2);
    CHECK_EQ(code_point, 0x00A4);
}

static void test_decode_invalid(void)
{
    uint16_t buffer;
    uint16_t code_point = 0xFFFF;

    buffer = 0;
    ((unsigned char *)&buffer)[0] = 0x80; /* stray high byte */
    CHECK_EQ(GB2312ToUTF16(buffer, &code_point), -1);

    buffer = 0;
    ((unsigned char *)&buffer)[0] = 0xA0; /* invalid lead */
    ((unsigned char *)&buffer)[1] = 0xA1;
    CHECK_EQ(GB2312ToUTF16(buffer, &code_point), -1);

    buffer = 0;
    ((unsigned char *)&buffer)[0] = 0xA1; /* invalid trail */
    ((unsigned char *)&buffer)[1] = 0xA0;
    CHECK_EQ(GB2312ToUTF16(buffer, &code_point), -1);

    buffer = 0;
    ((unsigned char *)&buffer)[0] = 0xAA; /* unassigned row, valid lead */
    ((unsigned char *)&buffer)[1] = 0xA1;
    CHECK_EQ(GB2312ToUTF16(buffer, &code_point), -1);

    buffer = 0;
    ((unsigned char *)&buffer)[0] = 0x41;
    CHECK_EQ(GB2312ToUTF16(buffer, NULL), -1);

    CHECK_EQ(code_point, 0xFFFF);
}

static void test_known_mappings(void)
{
    uint16_t buffer = 0;
    uint16_t code_point = 0;

    /* 0xB0A1 is the first GB2312 hanzi, U+554A. */
    buffer = 0;
    ((unsigned char *)&buffer)[0] = 0xB0;
    ((unsigned char *)&buffer)[1] = 0xA1;
    CHECK_EQ(GB2312ToUTF16(buffer, &code_point), 2);
    CHECK_EQ(code_point, 0x554A);
    CHECK_EQ(UTF16ToGB2312(0x554Au, &buffer), 2);
    CHECK_EQ(((const unsigned char *)&buffer)[0], 0xB0);
    CHECK_EQ(((const unsigned char *)&buffer)[1], 0xA1);
}

static void test_gbk_only_codes_are_rejected(void)
{
    uint16_t buffer = 0;
    uint16_t code_point = 0;

    /* U+20AC (euro sign) was added by GBK, not GB2312. */
    CHECK_EQ(UTF16ToGB2312(0x20ACu, &buffer), -1);
    /* Small roman numerals are a GBK extension as well. */
    CHECK_EQ(UTF16ToGB2312(0x2170u, &buffer), -1);

    buffer = 0;
    ((unsigned char *)&buffer)[0] = 0xA2;
    ((unsigned char *)&buffer)[1] = 0xE3;
    CHECK_EQ(GB2312ToUTF16(buffer, &code_point), -1);

    /* Legacy tables mapped these unassigned slots into the Private Use Area. */
    buffer = 0;
    ((unsigned char *)&buffer)[0] = 0xD7;
    ((unsigned char *)&buffer)[1] = 0xFA;
    CHECK_EQ(GB2312ToUTF16(buffer, &code_point), -1);
}

static void test_utf8_bridge(void)
{
    uint16_t buffer = 0;
    uint8_t utf8[4] = {0, 0, 0, 0};
    uint32_t consumed = 0;
    uint32_t gb2312_size = 0;
    static const uint8_t cjk[] = {0xE4, 0xB8, 0xAD};

    CHECK_EQ(UTF8ToGB2312((uint8_t *)cjk, &consumed, &buffer), 2);
    CHECK_EQ(consumed, 3);
    CHECK_EQ(((const unsigned char *)&buffer)[0], 0xD6);
    CHECK_EQ(((const unsigned char *)&buffer)[1], 0xD0);

    CHECK_EQ(GB2312ToUTF8(&buffer, &gb2312_size, utf8), 3);
    CHECK_EQ(gb2312_size, 2);
    CHECK_BYTES(utf8, cjk, 3);
}

static void test_utf8_bridge_errors(void)
{
    uint16_t buffer = 0;
    uint8_t utf8[4] = {0, 0, 0, 0};
    uint32_t size = 0;
    static const uint8_t emoji[] = {0xF0, 0x9F, 0x98, 0x80}; /* U+1F600 */
    static const uint8_t invalid[] = {0xC0, 0xAF, 0x00, 0x00};

    CHECK_EQ(UTF8ToGB2312((uint8_t *)emoji, &size, &buffer), -1);
    CHECK_EQ(UTF8ToGB2312((uint8_t *)invalid, &size, &buffer), -1);
    CHECK_EQ(UTF8ToGB2312(NULL, &size, &buffer), -1);
    CHECK_EQ(UTF8ToGB2312((uint8_t *)emoji, &size, NULL), -1);
    CHECK_EQ(GB2312ToUTF8(NULL, &size, utf8), -1);
    CHECK_EQ(GB2312ToUTF8(&buffer, &size, NULL), -1);
}

int main(void)
{
    test_encode_ascii();
    test_encode_cjk();
    test_latin_one_regression();
    test_encode_unmappable();
    test_decode();
    test_decode_invalid();
    test_known_mappings();
    test_gbk_only_codes_are_rejected();
    test_utf8_bridge();
    test_utf8_bridge_errors();
    TEST_REPORT();
}

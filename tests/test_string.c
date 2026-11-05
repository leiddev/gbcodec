/*
 * Buffer boundary and string conversion tests.
 *
 * Copyright (c) 2016-2026 leid
 * SPDX-License-Identifier: ISC
 */
#include "test_common.h"
#include "unicode_utility.h"

/* Sentinel used to detect writes past the logical capacity. */
#define SENTINEL 0xEE

static void fill(uint8_t *buffer, size_t size)
{
    memset(buffer, SENTINEL, size);
}

static void test_utf8_to_gb2312_capacity(void)
{
    static const uint8_t source[] = {0xE4, 0xB8, 0xAD, 0x00}; /* "中" */
    uint8_t buffer[8];

    /* Capacity 0: nothing may be written, not even the terminator. */
    fill(buffer, sizeof buffer);
    CHECK_EQ(UTF8ToGB2312Str((uint8_t *)source, buffer, 0), 0);
    CHECK_EQ(buffer[0], SENTINEL);

    /* Capacity 1: only the terminator fits. */
    fill(buffer, sizeof buffer);
    CHECK_EQ(UTF8ToGB2312Str((uint8_t *)source, buffer, 1), 0);
    CHECK_EQ(buffer[0], 0x00);
    CHECK_EQ(buffer[1], SENTINEL);

    /* Capacity 2: the character needs 2 bytes plus the terminator. This is
     * the regression case for the old off-by-one overflow. */
    fill(buffer, sizeof buffer);
    CHECK_EQ(UTF8ToGB2312Str((uint8_t *)source, buffer, 2), 0);
    CHECK_EQ(buffer[0], 0x00);
    CHECK_EQ(buffer[1], SENTINEL);
    CHECK_EQ(buffer[2], SENTINEL);

    /* Capacity 3: exact fit. */
    fill(buffer, sizeof buffer);
    CHECK_EQ(UTF8ToGB2312Str((uint8_t *)source, buffer, 3), 2);
    CHECK_EQ(buffer[0], 0xD6);
    CHECK_EQ(buffer[1], 0xD0);
    CHECK_EQ(buffer[2], 0x00);
    CHECK_EQ(buffer[3], SENTINEL);
}

static void test_gb2312_to_utf8_capacity(void)
{
    static const uint8_t source[] = {0xD6, 0xD0, 0x00}; /* "中" */
    uint8_t buffer[8];

    fill(buffer, sizeof buffer);
    CHECK_EQ(GB2312ToUTF8Str((uint8_t *)source, buffer, 0), 0);
    CHECK_EQ(buffer[0], SENTINEL);

    fill(buffer, sizeof buffer);
    CHECK_EQ(GB2312ToUTF8Str((uint8_t *)source, buffer, 3), 0);
    CHECK_EQ(buffer[0], 0x00);
    CHECK_EQ(buffer[1], SENTINEL);

    fill(buffer, sizeof buffer);
    CHECK_EQ(GB2312ToUTF8Str((uint8_t *)source, buffer, 4), 3);
    CHECK_EQ(buffer[3], 0x00);
    CHECK_EQ(buffer[4], SENTINEL);
}

static void test_ascii_boundary(void)
{
    static const uint8_t source[] = {0x41, 0x00};
    uint8_t buffer[4];

    fill(buffer, sizeof buffer);
    CHECK_EQ(UTF8ToGB2312Str((uint8_t *)source, buffer, 1), 0);
    CHECK_EQ(buffer[0], 0x00);

    fill(buffer, sizeof buffer);
    CHECK_EQ(UTF8ToGB2312Str((uint8_t *)source, buffer, 2), 1);
    CHECK_EQ(buffer[0], 0x41);
    CHECK_EQ(buffer[1], 0x00);
    CHECK_EQ(buffer[2], SENTINEL);
}

static void test_empty_input(void)
{
    uint8_t buffer[4];

    fill(buffer, sizeof buffer);
    CHECK_EQ(UTF8ToGB2312Str((uint8_t *)"", buffer, sizeof buffer), 0);
    CHECK_EQ(buffer[0], 0x00);

    fill(buffer, sizeof buffer);
    CHECK_EQ(GB2312ToUTF8Str((uint8_t *)"", buffer, sizeof buffer), 0);
    CHECK_EQ(buffer[0], 0x00);
}

static void test_round_trip(void)
{
    static const uint8_t source[] = {0xE4, 0xB8, 0xAD, 0xE6, 0x96, 0x87,
                                     0x61, 0x62, 0x63, 0x00}; /* "中文abc" */
    uint8_t gb2312[32];
    uint8_t utf8[32];
    int32_t gb_length;
    int32_t utf8_length;

    gb_length = UTF8ToGB2312Str((uint8_t *)source, gb2312, sizeof gb2312);
    CHECK_EQ(gb_length, 7);

    utf8_length = GB2312ToUTF8Str(gb2312, utf8, sizeof utf8);
    CHECK_EQ(utf8_length, 9);
    CHECK_BYTES(utf8, source, 9);
}

static void test_invalid_input_stops(void)
{
    static const uint8_t bad_utf8[] = {0x41, 0xFF, 0x42, 0x00};
    static const uint8_t bad_gb2312[] = {0x41, 0x80, 0x42, 0x00};
    static const uint8_t truncated_gb2312[] = {0x41, 0xD6, 0x00};
    uint8_t buffer[8];

    /* The valid prefix is converted, the invalid byte stops the loop. */
    fill(buffer, sizeof buffer);
    CHECK_EQ(UTF8ToGB2312Str((uint8_t *)bad_utf8, buffer, sizeof buffer), 1);
    CHECK_EQ(buffer[0], 0x41);
    CHECK_EQ(buffer[1], 0x00);

    fill(buffer, sizeof buffer);
    CHECK_EQ(GB2312ToUTF8Str((uint8_t *)bad_gb2312, buffer, sizeof buffer), 1);
    CHECK_EQ(buffer[0], 0x41);
    CHECK_EQ(buffer[1], 0x00);

    fill(buffer, sizeof buffer);
    CHECK_EQ(GB2312ToUTF8Str((uint8_t *)truncated_gb2312, buffer, sizeof buffer), 1);
    CHECK_EQ(buffer[0], 0x41);
    CHECK_EQ(buffer[1], 0x00);
}

static void test_null_arguments(void)
{
    uint8_t buffer[4];

    CHECK_EQ(UTF8ToGB2312Str(NULL, buffer, sizeof buffer), 0);
    CHECK_EQ(UTF8ToGB2312Str((uint8_t *)"A", NULL, sizeof buffer), 0);
    CHECK_EQ(GB2312ToUTF8Str(NULL, buffer, sizeof buffer), 0);
    CHECK_EQ(GB2312ToUTF8Str((uint8_t *)"A", NULL, sizeof buffer), 0);
}

int main(void)
{
    test_utf8_to_gb2312_capacity();
    test_gb2312_to_utf8_capacity();
    test_ascii_boundary();
    test_empty_input();
    test_round_trip();
    test_invalid_input_stops();
    test_null_arguments();
    TEST_REPORT();
}

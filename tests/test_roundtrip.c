/*
 * gbcodec - exhaustive round-trip tests.
 *
 * Copyright (c) 2016-2026 leid
 * SPDX-License-Identifier: ISC
 */
#include <gbcodec/convert.h>
#include <gbcodec/gb2312.h>

#include <stddef.h>
#include <stdint.h>

#include "test_common.h"

/* Worst cases: 8178 GB2312 byte pairs and three UTF-8 bytes per character. */
#define GBC_MAX_GB2312_BYTES (87u * 94u * 2u)
#define GBC_MAX_UTF8_BYTES (87u * 94u * 3u)

static uint8_t g_gb2312[GBC_MAX_GB2312_BYTES];
static uint8_t g_utf8[GBC_MAX_UTF8_BYTES];
static uint8_t g_roundtrip[GBC_MAX_GB2312_BYTES];

/* Builds one string containing every assigned GB2312 character. */
static size_t build_full_gb2312_string(void)
{
    size_t length = 0u;
    uint32_t lead;

    for (lead = 0xA1u; lead <= 0xF7u; ++lead) {
        uint32_t trail;

        for (trail = 0xA1u; trail <= 0xFEu; ++trail) {
            const uint8_t bytes[2] = {(uint8_t)lead, (uint8_t)trail};
            uint32_t code_point = 0u;
            size_t consumed = 0u;

            if (gbc_gb2312_decode(bytes, 2u, &code_point, &consumed) != GBC_OK) {
                continue;
            }
            g_gb2312[length] = bytes[0];
            g_gb2312[length + 1u] = bytes[1];
            length += 2u;
        }
    }

    return length;
}

static void test_full_roundtrip(void)
{
    size_t gb2312_length;
    size_t utf8_length = 0u;
    size_t back_length = 0u;

    gb2312_length = build_full_gb2312_string();
    CHECK_EQ(gb2312_length, 7445u * 2u);

    CHECK_EQ(gbc_gb2312_to_utf8_capacity(g_gb2312, gb2312_length, &utf8_length), GBC_OK);
    CHECK(utf8_length + 1u <= sizeof(g_utf8));

    CHECK_EQ(gbc_gb2312_to_utf8(g_gb2312, gb2312_length, g_utf8, sizeof(g_utf8), &utf8_length),
             GBC_OK);

    CHECK_EQ(gbc_utf8_to_gb2312(g_utf8, utf8_length, g_roundtrip, sizeof(g_roundtrip),
                                &back_length),
             GBC_OK);
    CHECK_EQ(back_length, gb2312_length);
    CHECK_BYTES(g_roundtrip, g_gb2312, gb2312_length);
}

static void test_ascii_roundtrip(void)
{
    static const uint8_t ascii[] = "The quick brown fox jumps over the lazy dog 0123456789";
    const size_t ascii_length = sizeof(ascii) - 1u;
    uint8_t buffer[sizeof(ascii)];
    size_t length = 0u;

    CHECK_EQ(gbc_utf8_to_gb2312(ascii, ascii_length, buffer, sizeof(buffer), &length), GBC_OK);
    CHECK_EQ(length, ascii_length);
    CHECK_BYTES(buffer, ascii, ascii_length);
    CHECK_EQ(buffer[ascii_length], 0u);
}

int main(void)
{
    test_full_roundtrip();
    test_ascii_roundtrip();
    return gbc_test_result();
}

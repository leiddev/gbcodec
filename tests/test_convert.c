/*
 * gbcodec - tests for the string conversion functions.
 *
 * Copyright (c) 2016-2026 leid
 * SPDX-License-Identifier: ISC
 */
#include <gbcodec/convert.h>

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "test_common.h"

#define GBC_SENTINEL 0xEEu

/* "中文abc" */
static const uint8_t UTF8_TEXT[] = {0xE4u, 0xB8u, 0xADu, 0xE6u, 0x96u, 0x87u,
                                    'a',   'b',   'c'};
/* "中文abc" in GB2312. */
static const uint8_t GB2312_TEXT[] = {0xD6u, 0xD0u, 0xCEu, 0xC4u, 'a', 'b', 'c'};

static void test_capacity_helpers(void)
{
    size_t length = 0u;

    CHECK_EQ(gbc_utf8_to_gb2312_capacity(UTF8_TEXT, sizeof(UTF8_TEXT), &length), GBC_OK);
    CHECK_EQ(length, sizeof(GB2312_TEXT));

    CHECK_EQ(gbc_gb2312_to_utf8_capacity(GB2312_TEXT, sizeof(GB2312_TEXT), &length), GBC_OK);
    CHECK_EQ(length, sizeof(UTF8_TEXT));

    CHECK_EQ(gbc_utf8_to_gb2312_capacity(NULL, 0u, &length), GBC_E_INVALID_ARGUMENT);
    CHECK_EQ(gbc_utf8_to_gb2312_capacity(UTF8_TEXT, sizeof(UTF8_TEXT), NULL),
             GBC_E_INVALID_ARGUMENT);

    /* Invalid input must be reported by the capacity helpers as well. */
    CHECK_EQ(gbc_utf8_to_gb2312_capacity(UTF8_TEXT, 1u, &length), GBC_E_TRUNCATED);
}

static void test_roundtrip(void)
{
    uint8_t encoded[16];
    uint8_t decoded[16];
    size_t encoded_length = 0u;
    size_t decoded_length = 0u;

    memset(encoded, GBC_SENTINEL, sizeof(encoded));
    memset(decoded, GBC_SENTINEL, sizeof(decoded));

    CHECK_EQ(gbc_utf8_to_gb2312(UTF8_TEXT, sizeof(UTF8_TEXT), encoded, sizeof(encoded),
                                &encoded_length),
             GBC_OK);
    CHECK_EQ(encoded_length, sizeof(GB2312_TEXT));
    CHECK_BYTES(encoded, GB2312_TEXT, sizeof(GB2312_TEXT));
    CHECK_EQ(encoded[sizeof(GB2312_TEXT)], 0u);

    CHECK_EQ(gbc_gb2312_to_utf8(GB2312_TEXT, sizeof(GB2312_TEXT), decoded, sizeof(decoded),
                                &decoded_length),
             GBC_OK);
    CHECK_EQ(decoded_length, sizeof(UTF8_TEXT));
    CHECK_BYTES(decoded, UTF8_TEXT, sizeof(UTF8_TEXT));
    CHECK_EQ(decoded[sizeof(UTF8_TEXT)], 0u);
}

static void test_buffer_too_small(void)
{
    uint8_t buffer[16];
    size_t length = 99u;

    /* The result needs seven bytes plus the terminator. */
    memset(buffer, GBC_SENTINEL, sizeof(buffer));
    CHECK_EQ(gbc_utf8_to_gb2312(UTF8_TEXT, sizeof(UTF8_TEXT), buffer, 0u, &length),
             GBC_E_BUFFER_TOO_SMALL);
    CHECK_EQ(length, 99u);

    memset(buffer, GBC_SENTINEL, sizeof(buffer));
    CHECK_EQ(gbc_utf8_to_gb2312(UTF8_TEXT, sizeof(UTF8_TEXT), buffer, sizeof(GB2312_TEXT),
                                &length),
             GBC_E_BUFFER_TOO_SMALL);
    CHECK_EQ(length, 99u);
    CHECK_EQ(buffer[0], 0u);
    CHECK_EQ(buffer[1], GBC_SENTINEL);
    CHECK_EQ(buffer[sizeof(buffer) - 1u], GBC_SENTINEL);

    /* The exact size is accepted and the rest of the buffer is untouched. */
    memset(buffer, GBC_SENTINEL, sizeof(buffer));
    CHECK_EQ(gbc_utf8_to_gb2312(UTF8_TEXT, sizeof(UTF8_TEXT), buffer, sizeof(GB2312_TEXT) + 1u,
                                &length),
             GBC_OK);
    CHECK_EQ(length, sizeof(GB2312_TEXT));
    CHECK_EQ(buffer[sizeof(GB2312_TEXT)], 0u);
    CHECK_EQ(buffer[sizeof(GB2312_TEXT) + 1u], GBC_SENTINEL);
}

static void test_empty_input(void)
{
    uint8_t buffer[4];
    size_t length = 99u;

    memset(buffer, GBC_SENTINEL, sizeof(buffer));
    CHECK_EQ(gbc_utf8_to_gb2312(UTF8_TEXT, 0u, buffer, sizeof(buffer), &length), GBC_OK);
    CHECK_EQ(length, 0u);
    CHECK_EQ(buffer[0], 0u);
    CHECK_EQ(buffer[1], GBC_SENTINEL);

    /* An empty result needs no room for a terminator either. */
    length = 99u;
    CHECK_EQ(gbc_utf8_to_gb2312(UTF8_TEXT, 0u, buffer, 0u, &length), GBC_OK);
    CHECK_EQ(length, 0u);
}

static void test_embedded_nul(void)
{
    static const uint8_t input[] = {'A', 0x00u, 'B'};
    uint8_t buffer[8];
    size_t length = 0u;

    memset(buffer, GBC_SENTINEL, sizeof(buffer));
    CHECK_EQ(gbc_utf8_to_gb2312(input, sizeof(input), buffer, sizeof(buffer), &length), GBC_OK);
    CHECK_EQ(length, sizeof(input));
    CHECK_BYTES(buffer, input, sizeof(input));
    CHECK_EQ(buffer[sizeof(input)], 0u);
}

static void test_failures(void)
{
    static const uint8_t invalid_utf8[] = {0xFFu};
    static const uint8_t unmappable_utf8[] = {0xE2u, 0x82u, 0xACu}; /* U+20AC */
    static const uint8_t invalid_gb2312[] = {0x81u};
    uint8_t buffer[8];
    size_t length = 99u;

    memset(buffer, GBC_SENTINEL, sizeof(buffer));
    CHECK_EQ(gbc_utf8_to_gb2312(invalid_utf8, sizeof(invalid_utf8), buffer, sizeof(buffer),
                                &length),
             GBC_E_INVALID_SEQUENCE);
    CHECK_EQ(length, 99u);
    CHECK_EQ(buffer[0], 0u);

    memset(buffer, GBC_SENTINEL, sizeof(buffer));
    CHECK_EQ(gbc_utf8_to_gb2312(unmappable_utf8, sizeof(unmappable_utf8), buffer,
                                sizeof(buffer), &length),
             GBC_E_UNMAPPABLE);
    CHECK_EQ(length, 99u);
    CHECK_EQ(buffer[0], 0u);

    memset(buffer, GBC_SENTINEL, sizeof(buffer));
    CHECK_EQ(gbc_gb2312_to_utf8(invalid_gb2312, sizeof(invalid_gb2312), buffer, sizeof(buffer),
                                &length),
             GBC_E_INVALID_SEQUENCE);
    CHECK_EQ(length, 99u);
    CHECK_EQ(buffer[0], 0u);

    CHECK_EQ(gbc_utf8_to_gb2312(NULL, 1u, buffer, sizeof(buffer), &length),
             GBC_E_INVALID_ARGUMENT);
    CHECK_EQ(gbc_utf8_to_gb2312(UTF8_TEXT, sizeof(UTF8_TEXT), NULL, 0u, &length),
             GBC_E_INVALID_ARGUMENT);
    CHECK_EQ(gbc_gb2312_to_utf8(NULL, 1u, buffer, sizeof(buffer), &length),
             GBC_E_INVALID_ARGUMENT);
    CHECK_EQ(gbc_gb2312_to_utf8(GB2312_TEXT, sizeof(GB2312_TEXT), NULL, 0u, &length),
             GBC_E_INVALID_ARGUMENT);
}

static void test_nul_terminated_variants(void)
{
    char encoded[16];
    char decoded[16];
    size_t encoded_length = 0u;
    size_t decoded_length = 0u;
    const char utf8_text[] = "\xE4\xB8\xAD\xE6\x96\x87" "abc";
    const char gb2312_text[] = "\xD6\xD0\xCE\xC4" "abc";

    memset(encoded, (char)GBC_SENTINEL, sizeof(encoded));
    CHECK_EQ(gbc_utf8_to_gb2312_z(utf8_text, encoded, sizeof(encoded), &encoded_length), GBC_OK);
    CHECK_EQ(encoded_length, sizeof(GB2312_TEXT));
    CHECK_BYTES(encoded, GB2312_TEXT, sizeof(GB2312_TEXT));
    CHECK_EQ(encoded[sizeof(GB2312_TEXT)], 0);

    memset(decoded, (char)GBC_SENTINEL, sizeof(decoded));
    CHECK_EQ(gbc_gb2312_to_utf8_z(gb2312_text, decoded, sizeof(decoded), &decoded_length), GBC_OK);
    CHECK_EQ(decoded_length, sizeof(UTF8_TEXT));
    CHECK_BYTES(decoded, UTF8_TEXT, sizeof(UTF8_TEXT));
    CHECK_EQ(decoded[sizeof(UTF8_TEXT)], 0);

    CHECK_EQ(gbc_utf8_to_gb2312_z(NULL, encoded, sizeof(encoded), &encoded_length),
             GBC_E_INVALID_ARGUMENT);
    CHECK_EQ(gbc_gb2312_to_utf8_z(NULL, decoded, sizeof(decoded), &decoded_length),
             GBC_E_INVALID_ARGUMENT);
}

int main(void)
{
    test_capacity_helpers();
    test_roundtrip();
    test_buffer_too_small();
    test_empty_input();
    test_embedded_nul();
    test_failures();
    test_nul_terminated_variants();
    return gbc_test_result();
}

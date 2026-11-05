/*
 * gbcodec - tests for the lossy (substituting) conversion functions.
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

/* "中文😀abc" */
static const uint8_t UTF8_EMOJI[] = {0xE4u, 0xB8u, 0xADu, 0xE6u, 0x96u, 0x87u, 0xF0u,
                                     0x9Fu, 0x98u, 0x80u, 'a',   'b',   'c'};
/* "中文?abc" */
static const uint8_t GB2312_QUESTION[] = {0xD6u, 0xD0u, 0xCEu, 0xC4u, '?', 'a', 'b', 'c'};
/* "中文？abc" */
static const uint8_t GB2312_FULLWIDTH_QUESTION[] = {0xD6u, 0xD0u, 0xCEu, 0xC4u,
                                                    0xA3u, 0xBFu, 'a',   'b',
                                                    'c'};
/* "中文abc" */
static const uint8_t GB2312_PLAIN[] = {0xD6u, 0xD0u, 0xCEu, 0xC4u, 'a', 'b', 'c'};

/* "中文" followed by an unassigned pair and "文". */
static const uint8_t GB2312_HOLE[] = {0xD6u, 0xD0u, 0xAAu, 0xA1u, 0xCEu, 0xC4u};
/* "中" + U+FFFD + "文" */
static const uint8_t UTF8_REPLACEMENT[] = {0xE4u, 0xB8u, 0xADu, 0xEFu, 0xBFu,
                                           0xBDu, 0xE6u, 0x96u, 0x87u};
/* "中?文" */
static const uint8_t UTF8_QUESTION[] = {0xE4u, 0xB8u, 0xADu, '?', 0xE6u, 0x96u, 0x87u};

static void test_replace_into_gb2312(void)
{
    uint8_t buffer[32];
    size_t length = 99u;
    size_t replaced = 99u;

    memset(buffer, GBC_SENTINEL, sizeof(buffer));
    CHECK_EQ(gbc_utf8_to_gb2312_replace(UTF8_EMOJI, sizeof(UTF8_EMOJI), buffer, sizeof(buffer), '?',
                                        &length, &replaced),
             GBC_OK);
    CHECK_EQ(length, sizeof(GB2312_QUESTION));
    CHECK_EQ(replaced, 1u);
    CHECK_BYTES(buffer, GB2312_QUESTION, sizeof(GB2312_QUESTION));
    CHECK_EQ(buffer[sizeof(GB2312_QUESTION)], 0u);

    /* The replacement itself may be a GB2312 character. */
    memset(buffer, GBC_SENTINEL, sizeof(buffer));
    CHECK_EQ(gbc_utf8_to_gb2312_replace(UTF8_EMOJI, sizeof(UTF8_EMOJI), buffer, sizeof(buffer),
                                        0xFF1Fu /* ？ */, &length, &replaced),
             GBC_OK);
    CHECK_EQ(length, sizeof(GB2312_FULLWIDTH_QUESTION));
    CHECK_BYTES(buffer, GB2312_FULLWIDTH_QUESTION, sizeof(GB2312_FULLWIDTH_QUESTION));
    CHECK_EQ(buffer[sizeof(GB2312_FULLWIDTH_QUESTION)], 0u);

    /* A zero replacement deletes the character. */
    memset(buffer, GBC_SENTINEL, sizeof(buffer));
    CHECK_EQ(gbc_utf8_to_gb2312_replace(UTF8_EMOJI, sizeof(UTF8_EMOJI), buffer, sizeof(buffer), 0u,
                                        &length, &replaced),
             GBC_OK);
    CHECK_EQ(length, sizeof(GB2312_PLAIN));
    CHECK_EQ(replaced, 1u);
    CHECK_BYTES(buffer, GB2312_PLAIN, sizeof(GB2312_PLAIN));

    /* The output never exceeds the input, so src_length + 1 is enough. */
    memset(buffer, GBC_SENTINEL, sizeof(buffer));
    CHECK_EQ(gbc_utf8_to_gb2312_replace(UTF8_EMOJI, sizeof(UTF8_EMOJI), buffer,
                                        sizeof(UTF8_EMOJI) + 1u, '?', &length, &replaced),
             GBC_OK);
    CHECK_BYTES(buffer, GB2312_QUESTION, sizeof(GB2312_QUESTION));

    /* Both out parameters are optional. */
    memset(buffer, GBC_SENTINEL, sizeof(buffer));
    CHECK_EQ(gbc_utf8_to_gb2312_replace(UTF8_EMOJI, sizeof(UTF8_EMOJI), buffer, sizeof(buffer), '?',
                                        NULL, NULL),
             GBC_OK);
    CHECK_BYTES(buffer, GB2312_QUESTION, sizeof(GB2312_QUESTION));
}

static void test_replace_into_utf8(void)
{
    uint8_t buffer[32];
    size_t length = 99u;
    size_t replaced = 99u;

    memset(buffer, GBC_SENTINEL, sizeof(buffer));
    CHECK_EQ(gbc_gb2312_to_utf8_replace(GB2312_HOLE, sizeof(GB2312_HOLE), buffer, sizeof(buffer),
                                        0xFFFDu, &length, &replaced),
             GBC_OK);
    CHECK_EQ(length, sizeof(UTF8_REPLACEMENT));
    CHECK_EQ(replaced, 1u);
    CHECK_BYTES(buffer, UTF8_REPLACEMENT, sizeof(UTF8_REPLACEMENT));
    CHECK_EQ(buffer[sizeof(UTF8_REPLACEMENT)], 0u);

    memset(buffer, GBC_SENTINEL, sizeof(buffer));
    CHECK_EQ(gbc_gb2312_to_utf8_replace(GB2312_HOLE, sizeof(GB2312_HOLE), buffer, sizeof(buffer),
                                        '?', &length, &replaced),
             GBC_OK);
    CHECK_EQ(length, sizeof(UTF8_QUESTION));
    CHECK_BYTES(buffer, UTF8_QUESTION, sizeof(UTF8_QUESTION));

    /* Deletion. */
    memset(buffer, GBC_SENTINEL, sizeof(buffer));
    CHECK_EQ(gbc_gb2312_to_utf8_replace(GB2312_HOLE, sizeof(GB2312_HOLE), buffer, sizeof(buffer),
                                        0u, &length, &replaced),
             GBC_OK);
    CHECK_EQ(length, 6u); /* "中文" (3 + 3) */
    CHECK_EQ(replaced, 1u);

    /* A pair expands to at most three bytes, so 3 * src_length + 1 is enough. */
    CHECK_EQ(gbc_gb2312_to_utf8_replace(GB2312_HOLE, sizeof(GB2312_HOLE), buffer,
                                        3u * sizeof(GB2312_HOLE) + 1u, 0xFFFDu, &length, &replaced),
             GBC_OK);
    CHECK_BYTES(buffer, UTF8_REPLACEMENT, sizeof(UTF8_REPLACEMENT));
}

static void test_everything_replaced(void)
{
    static const uint8_t emoji_only[] = {0xF0u, 0x9Fu, 0x98u, 0x80u};
    uint8_t buffer[8];
    size_t length = 99u;
    size_t replaced = 99u;

    /* Deleting every character yields an empty result, which needs no room. */
    memset(buffer, GBC_SENTINEL, sizeof(buffer));
    CHECK_EQ(gbc_utf8_to_gb2312_replace(emoji_only, sizeof(emoji_only), buffer, 0u, 0u, &length,
                                        &replaced),
             GBC_OK);
    CHECK_EQ(length, 0u);
    CHECK_EQ(replaced, 1u);
    CHECK_EQ(buffer[0], GBC_SENTINEL);

    CHECK_EQ(gbc_utf8_to_gb2312_replace(emoji_only, sizeof(emoji_only), buffer, sizeof(buffer), '?',
                                        &length, &replaced),
             GBC_OK);
    CHECK_EQ(length, 1u);
    CHECK_EQ(replaced, 1u);
    CHECK_EQ(buffer[0], '?');
    CHECK_EQ(buffer[1], 0u);
}

static void test_invalid_replacement(void)
{
    static const uint32_t bad_gb2312[] = {0x20ACu /* € */, 0x1F600u /* 😀 */, 0xD800u};
    static const uint32_t bad_utf8[] = {0xD800u, 0x110000u};
    uint8_t buffer[16];
    size_t length = 99u;
    size_t replaced = 99u;
    size_t index;

    for (index = 0u; index < sizeof(bad_gb2312) / sizeof(bad_gb2312[0]); ++index) {
        memset(buffer, GBC_SENTINEL, sizeof(buffer));
        length = 99u;
        replaced = 99u;
        CHECK_EQ(gbc_utf8_to_gb2312_replace(UTF8_EMOJI, sizeof(UTF8_EMOJI), buffer, sizeof(buffer),
                                            bad_gb2312[index], &length, &replaced),
                 GBC_E_INVALID_ARGUMENT);
        CHECK_EQ(length, 99u);
        CHECK_EQ(replaced, 99u);
        CHECK_EQ(buffer[0], 0u);
    }

    for (index = 0u; index < sizeof(bad_utf8) / sizeof(bad_utf8[0]); ++index) {
        memset(buffer, GBC_SENTINEL, sizeof(buffer));
        length = 99u;
        replaced = 99u;
        CHECK_EQ(gbc_gb2312_to_utf8_replace(GB2312_HOLE, sizeof(GB2312_HOLE), buffer,
                                            sizeof(buffer), bad_utf8[index], &length, &replaced),
                 GBC_E_INVALID_ARGUMENT);
        CHECK_EQ(length, 99u);
        CHECK_EQ(replaced, 99u);
        CHECK_EQ(buffer[0], 0u);
    }

    /* The replacement is validated even when there is nothing to convert. */
    length = 99u;
    CHECK_EQ(gbc_utf8_to_gb2312_replace(UTF8_EMOJI, 0u, buffer, sizeof(buffer), 0x20ACu, &length,
                                        &replaced),
             GBC_E_INVALID_ARGUMENT);
}

static void test_malformed_input_is_not_replaced(void)
{
    static const uint8_t invalid_utf8[] = {0xFFu};
    static const uint8_t truncated_utf8[] = {0xE4u};
    static const uint8_t invalid_gb2312[] = {0x81u};
    static const uint8_t truncated_gb2312[] = {0xD6u};
    uint8_t buffer[16];
    size_t length = 99u;
    size_t replaced = 99u;

    memset(buffer, GBC_SENTINEL, sizeof(buffer));
    CHECK_EQ(gbc_utf8_to_gb2312_replace(invalid_utf8, sizeof(invalid_utf8), buffer,
                                        sizeof(buffer), '?', &length, &replaced),
             GBC_E_INVALID_SEQUENCE);
    CHECK_EQ(length, 99u);
    CHECK_EQ(replaced, 99u);
    CHECK_EQ(buffer[0], 0u);

    memset(buffer, GBC_SENTINEL, sizeof(buffer));
    CHECK_EQ(gbc_utf8_to_gb2312_replace(truncated_utf8, sizeof(truncated_utf8), buffer,
                                        sizeof(buffer), '?', &length, &replaced),
             GBC_E_TRUNCATED);
    CHECK_EQ(buffer[0], 0u);

    memset(buffer, GBC_SENTINEL, sizeof(buffer));
    CHECK_EQ(gbc_gb2312_to_utf8_replace(invalid_gb2312, sizeof(invalid_gb2312), buffer,
                                        sizeof(buffer), 0xFFFDu, &length, &replaced),
             GBC_E_INVALID_SEQUENCE);
    CHECK_EQ(buffer[0], 0u);

    memset(buffer, GBC_SENTINEL, sizeof(buffer));
    CHECK_EQ(gbc_gb2312_to_utf8_replace(truncated_gb2312, sizeof(truncated_gb2312), buffer,
                                        sizeof(buffer), 0xFFFDu, &length, &replaced),
             GBC_E_TRUNCATED);
    CHECK_EQ(buffer[0], 0u);
}

static void test_buffer_too_small(void)
{
    uint8_t buffer[16];
    size_t length = 99u;
    size_t replaced = 99u;

    /* The replaced result needs eight bytes plus the terminator. */
    memset(buffer, GBC_SENTINEL, sizeof(buffer));
    CHECK_EQ(gbc_utf8_to_gb2312_replace(UTF8_EMOJI, sizeof(UTF8_EMOJI), buffer,
                                        sizeof(GB2312_QUESTION), '?', &length, &replaced),
             GBC_E_BUFFER_TOO_SMALL);
    CHECK_EQ(length, 99u);
    CHECK_EQ(replaced, 99u);
    CHECK_EQ(buffer[0], 0u);
    CHECK_EQ(buffer[1], GBC_SENTINEL);

    memset(buffer, GBC_SENTINEL, sizeof(buffer));
    CHECK_EQ(gbc_utf8_to_gb2312_replace(UTF8_EMOJI, sizeof(UTF8_EMOJI), buffer,
                                        sizeof(GB2312_QUESTION) + 1u, '?', &length, &replaced),
             GBC_OK);
    CHECK_EQ(length, sizeof(GB2312_QUESTION));
    CHECK_EQ(buffer[sizeof(GB2312_QUESTION)], 0u);
    CHECK_EQ(buffer[sizeof(GB2312_QUESTION) + 1u], GBC_SENTINEL);
}

static void test_empty_and_null_arguments(void)
{
    uint8_t buffer[8];
    size_t length = 99u;
    size_t replaced = 99u;

    memset(buffer, GBC_SENTINEL, sizeof(buffer));
    CHECK_EQ(gbc_utf8_to_gb2312_replace(UTF8_EMOJI, 0u, buffer, sizeof(buffer), '?', &length,
                                        &replaced),
             GBC_OK);
    CHECK_EQ(length, 0u);
    CHECK_EQ(replaced, 0u);
    CHECK_EQ(buffer[0], 0u);

    CHECK_EQ(gbc_utf8_to_gb2312_replace(NULL, 1u, buffer, sizeof(buffer), '?', &length, &replaced),
             GBC_E_INVALID_ARGUMENT);
    CHECK_EQ(gbc_utf8_to_gb2312_replace(UTF8_EMOJI, sizeof(UTF8_EMOJI), NULL, 0u, '?', &length,
                                        &replaced),
             GBC_E_INVALID_ARGUMENT);
    CHECK_EQ(gbc_gb2312_to_utf8_replace(NULL, 1u, buffer, sizeof(buffer), 0xFFFDu, &length,
                                        &replaced),
             GBC_E_INVALID_ARGUMENT);
}

static void test_nul_terminated_variants(void)
{
    char buffer[32];
    size_t length = 99u;
    size_t replaced = 99u;
    const char utf8_text[] = "\xE4\xB8\xAD\xE6\x96\x87\xF0\x9F\x98\x80" "abc";
    const char gb2312_text[] = "\xD6\xD0\xAA\xA1\xCE\xC4";

    memset(buffer, (char)GBC_SENTINEL, sizeof(buffer));
    CHECK_EQ(gbc_utf8_to_gb2312_replace_z(utf8_text, buffer, sizeof(buffer), '?', &length,
                                          &replaced),
             GBC_OK);
    CHECK_EQ(length, sizeof(GB2312_QUESTION));
    CHECK_EQ(replaced, 1u);
    CHECK_BYTES(buffer, GB2312_QUESTION, sizeof(GB2312_QUESTION));
    CHECK_EQ(buffer[sizeof(GB2312_QUESTION)], 0);

    memset(buffer, (char)GBC_SENTINEL, sizeof(buffer));
    CHECK_EQ(gbc_gb2312_to_utf8_replace_z(gb2312_text, buffer, sizeof(buffer), 0xFFFDu, &length,
                                          &replaced),
             GBC_OK);
    CHECK_EQ(length, sizeof(UTF8_REPLACEMENT));
    CHECK_BYTES(buffer, UTF8_REPLACEMENT, sizeof(UTF8_REPLACEMENT));

    CHECK_EQ(gbc_utf8_to_gb2312_replace_z(NULL, buffer, sizeof(buffer), '?', &length, &replaced),
             GBC_E_INVALID_ARGUMENT);
    CHECK_EQ(gbc_gb2312_to_utf8_replace_z(NULL, buffer, sizeof(buffer), 0xFFFDu, &length,
                                          &replaced),
             GBC_E_INVALID_ARGUMENT);
}

int main(void)
{
    test_replace_into_gb2312();
    test_replace_into_utf8();
    test_everything_replaced();
    test_invalid_replacement();
    test_malformed_input_is_not_replaced();
    test_buffer_too_small();
    test_empty_and_null_arguments();
    test_nul_terminated_variants();
    return gbc_test_result();
}

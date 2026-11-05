/*
 * gbcodec - basic C usage example.
 *
 * Copyright (c) 2016-2026 leid
 * SPDX-License-Identifier: ISC
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "unicode_utility.h"

static void print_hex(const char *label, const uint8_t *data, size_t length)
{
    size_t index;

    printf("%s:", label);
    for (index = 0; index < length; ++index) {
        printf(" %02X", data[index]);
    }
    printf("\n");
}

int main(void)
{
    /* "中文abc" in UTF-8. */
    static const uint8_t utf8_text[] = {0xE4, 0xB8, 0xAD, 0xE6, 0x96, 0x87,
                                        0x61, 0x62, 0x63, 0x00};
    uint8_t gb2312[32];
    uint8_t utf8[32];
    int32_t gb_length;
    int32_t utf8_length;

    gb_length = UTF8ToGB2312Str((uint8_t *)utf8_text, gb2312, sizeof gb2312);
    if (gb_length <= 0) {
        fprintf(stderr, "UTF-8 to GB2312 conversion failed\n");
        return 1;
    }
    print_hex("UTF-8  ", utf8_text, strlen((const char *)utf8_text));
    print_hex("GB2312 ", gb2312, (size_t)gb_length);

    utf8_length = GB2312ToUTF8Str(gb2312, utf8, sizeof utf8);
    if (utf8_length <= 0) {
        fprintf(stderr, "GB2312 to UTF-8 conversion failed\n");
        return 1;
    }
    print_hex("UTF-8  ", utf8, (size_t)utf8_length);

    return 0;
}

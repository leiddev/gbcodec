/*
 * gbcodec - basic C example.
 *
 * Copyright (c) 2016-2026 leid
 * SPDX-License-Identifier: ISC
 */
#include <gbcodec/gbcodec.h>

#include <stdio.h>
#include <string.h>

int main(void)
{
    /* "中文abc" in UTF-8. */
    static const char utf8_text[] = "\xE4\xB8\xAD\xE6\x96\x87" "abc";
    char gb2312[16];
    char decoded[32];
    size_t gb2312_length = 0u;
    size_t decoded_length = 0u;
    size_t index;

    if (gbc_utf8_to_gb2312_z(utf8_text, gb2312, sizeof(gb2312), &gb2312_length) != GBC_OK) {
        fprintf(stderr, "UTF-8 to GB2312 conversion failed\n");
        return 1;
    }

    printf("GB2312 (%zu bytes):", gb2312_length);
    for (index = 0u; index < gb2312_length; ++index) {
        printf(" %02X", (unsigned)(unsigned char)gb2312[index]);
    }
    printf("\n");

    if (gbc_gb2312_to_utf8_z(gb2312, decoded, sizeof(decoded), &decoded_length) != GBC_OK) {
        fprintf(stderr, "GB2312 to UTF-8 conversion failed\n");
        return 1;
    }

    printf("UTF-8 (%zu bytes): %s\n", decoded_length, decoded);
    return strcmp(utf8_text, decoded) == 0 ? 0 : 1;
}

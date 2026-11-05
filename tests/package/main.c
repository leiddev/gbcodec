/*
 * gbcodec - installed-package smoke test (C).
 *
 * Copyright (c) 2016-2026 leid
 * SPDX-License-Identifier: ISC
 */
#include <gbcodec/gbcodec.h>

#include <stdio.h>
#include <string.h>

int main(void)
{
    static const char utf8_text[] = "\xE4\xB8\xAD\xE6\x96\x87" "abc";
    char gb2312[16];
    char decoded[32];
    size_t gb2312_length = 0u;
    size_t decoded_length = 0u;

    if (gbc_utf8_to_gb2312_z(utf8_text, gb2312, sizeof(gb2312), &gb2312_length) != GBC_OK) {
        return 1;
    }
    if (gbc_gb2312_to_utf8_z(gb2312, decoded, sizeof(decoded), &decoded_length) != GBC_OK) {
        return 1;
    }

    printf("gbcodec %s: %zu -> %zu bytes\n", GBCODEC_VERSION_STRING, gb2312_length,
           decoded_length);
    return strcmp(utf8_text, decoded) == 0 ? 0 : 1;
}

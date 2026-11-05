/*
 * Temporary smoke test used during the transition to 1.0.
 * It is replaced by the full unit-test suite in the next milestone.
 */
#include <stdint.h>
#include <stdio.h>

#include "unicode_utility.h"

int main(void)
{
    static const uint8_t utf8_cjk[] = {0xE4, 0xB8, 0xAD}; /* U+4E2D, "中" */
    uint32_t consumed = 0;
    uint16_t gb2312 = 0;
    uint16_t roundtrip = 0;

    if (UTF8ToGB2312((uint8_t *)utf8_cjk, &consumed, &gb2312) <= 0) {
        fprintf(stderr, "smoke: UTF8ToGB2312() failed\n");
        return 1;
    }
    if (consumed != sizeof(utf8_cjk)) {
        fprintf(stderr, "smoke: unexpected consumed length %u\n", consumed);
        return 2;
    }
    /* U+4E2D is 0xD6D0 in GB2312; the value is endian independent. */
    if (gb2312 != 0xD6D0) {
        fprintf(stderr, "smoke: unexpected GB2312 code 0x%04X\n", gb2312);
        return 3;
    }
    if (GB2312ToUTF16(gb2312, &roundtrip) <= 0 || roundtrip != 0x4E2D) {
        fprintf(stderr, "smoke: round trip failed (0x%04X)\n", roundtrip);
        return 4;
    }
    return 0;
}

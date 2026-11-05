/*
 * gbcodec - GB2312 <-> Unicode (UTF-8 / UTF-16) conversion library.
 *
 * Copyright (c) 2016-2026 leid
 * SPDX-License-Identifier: ISC
 */
#include "unicode_utility.h"

#include <stddef.h>
#include <string.h>

#include "detail/gb2312_table.h"

#define GBC_ASCII_LIMIT 0x80u
#define GBC_SURROGATE_FIRST 0xD800u
#define GBC_SURROGATE_LAST 0xDFFFu
#define GBC_UNICODE_MAX 0x10FFFFu

static int gbc_is_utf8_continuation(uint8_t byte)
{
    return (byte & 0xC0u) == 0x80u;
}

static int gbc_is_gb2312_lead(uint8_t byte)
{
    return byte >= 0xA1u && byte <= 0xF7u;
}

static int gbc_is_gb2312_trail(uint8_t byte)
{
    return byte >= 0xA1u && byte <= 0xFEu;
}

/*
 * O(log n) lookup by code point: binary search the sorted table.
 * Returns 0 when the code point has no GB2312 mapping.
 */
static uint16_t gbc_lookup_by_code_point(uint16_t code_point)
{
    uint32_t low = 0;
    uint32_t high = GBC_TABLE_ENTRIES;

    while (low < high) {
        const uint32_t middle = low + (high - low) / 2u;
        const uint16_t candidate = gbc_detail_code_point_order[middle].code_point;

        if (candidate < code_point) {
            low = middle + 1u;
        } else {
            high = middle;
        }
    }

    if (low < GBC_TABLE_ENTRIES && gbc_detail_code_point_order[low].code_point == code_point) {
        return gbc_detail_code_point_order[low].gb2312;
    }
    return 0;
}

/*
 * O(1) lookup by GB2312 code: index the flat table directly.
 * The caller must have validated the lead and trail byte ranges.
 * Returns 0 when the code is not assigned.
 */
static uint16_t gbc_lookup_by_gb2312(uint16_t gb2312)
{
    const uint32_t lead = (uint32_t)((gb2312 >> 8) & 0xFFu);
    const uint32_t trail = (uint32_t)(gb2312 & 0xFFu);
    const uint32_t index = (lead - GBC_GB2312_FIRST_LEAD) * GBC_GB2312_TRAILS_PER_LEAD +
                           (trail - GBC_GB2312_FIRST_TRAIL);

    return gbc_detail_gb2312_index[index];
}

/*
 * Decode one GB2312 character from a byte stream. Returns the number of bytes
 * consumed (1 or 2) and stores the code point, or returns -1 on invalid input.
 */
static int32_t gbc_decode_gb2312(const uint8_t *bytes, uint16_t *code_point)
{
    uint16_t mapped;

    if (bytes[0] < GBC_ASCII_LIMIT) {
        *code_point = bytes[0];
        return 1;
    }
    if (!gbc_is_gb2312_lead(bytes[0]) || !gbc_is_gb2312_trail(bytes[1])) {
        return -1;
    }

    mapped = gbc_lookup_by_gb2312((uint16_t)(((uint16_t)bytes[0] << 8) | bytes[1]));
    if (mapped == 0) {
        return -1;
    }
    *code_point = mapped;
    return 2;
}

/*
 * Encode one BMP code point as GB2312 into a two-byte buffer. Returns the
 * number of bytes written (1 or 2), or -1 when the code point is unmappable.
 */
static int32_t gbc_encode_gb2312(uint16_t code_point, uint8_t *out)
{
    uint16_t mapped;

    if (code_point < GBC_ASCII_LIMIT) {
        out[0] = (uint8_t)code_point;
        return 1;
    }

    mapped = gbc_lookup_by_code_point(code_point);
    if (mapped == 0) {
        return -1;
    }
    out[0] = (uint8_t)(mapped >> 8);
    out[1] = (uint8_t)(mapped & 0xFFu);
    return 2;
}

int32_t UTF8ToUnicode(uint8_t *utf8, uint32_t *unicode)
{
    uint8_t lead;
    uint32_t code_point;
    int32_t length;
    int32_t index;

    if (utf8 == NULL || unicode == NULL) {
        return -1;
    }

    lead = utf8[0];
    if (lead < GBC_ASCII_LIMIT) {
        *unicode = lead;
        return 1;
    }

    if (lead >= 0xC2u && lead <= 0xDFu) {
        length = 2;
        code_point = (uint32_t)(lead & 0x1Fu);
    } else if (lead >= 0xE0u && lead <= 0xEFu) {
        length = 3;
        code_point = (uint32_t)(lead & 0x0Fu);
    } else if (lead >= 0xF0u && lead <= 0xF4u) {
        length = 4;
        code_point = (uint32_t)(lead & 0x07u);
    } else {
        /* Continuation byte, overlong lead (0xC0/0xC1) or invalid lead. */
        return -1;
    }

    for (index = 1; index < length; ++index) {
        uint8_t byte = utf8[index];
        if (!gbc_is_utf8_continuation(byte)) {
            return -1;
        }
        code_point = (code_point << 6) | (uint32_t)(byte & 0x3Fu);
    }

    /* Reject overlong forms, surrogates and out-of-range code points. */
    if (length == 2 && code_point < 0x80u) {
        return -1;
    }
    if (length == 3 && code_point < 0x800u) {
        return -1;
    }
    if (length == 4 && code_point < 0x10000u) {
        return -1;
    }
    if (code_point > GBC_UNICODE_MAX) {
        return -1;
    }
    if (code_point >= GBC_SURROGATE_FIRST && code_point <= GBC_SURROGATE_LAST) {
        return -1;
    }

    *unicode = code_point;
    return length;
}

int32_t UnicodeToUTF8(uint32_t unicodevalue, uint8_t *utf8dst)
{
    if (utf8dst == NULL) {
        return 0;
    }
    if (unicodevalue > GBC_UNICODE_MAX) {
        return 0;
    }
    if (unicodevalue >= GBC_SURROGATE_FIRST && unicodevalue <= GBC_SURROGATE_LAST) {
        return 0;
    }

    if (unicodevalue < 0x80u) {
        utf8dst[0] = (uint8_t)unicodevalue;
        return 1;
    }
    if (unicodevalue < 0x800u) {
        utf8dst[0] = (uint8_t)(0xC0u | (unicodevalue >> 6));
        utf8dst[1] = (uint8_t)(0x80u | (unicodevalue & 0x3Fu));
        return 2;
    }
    if (unicodevalue < 0x10000u) {
        utf8dst[0] = (uint8_t)(0xE0u | (unicodevalue >> 12));
        utf8dst[1] = (uint8_t)(0x80u | ((unicodevalue >> 6) & 0x3Fu));
        utf8dst[2] = (uint8_t)(0x80u | (unicodevalue & 0x3Fu));
        return 3;
    }

    utf8dst[0] = (uint8_t)(0xF0u | (unicodevalue >> 18));
    utf8dst[1] = (uint8_t)(0x80u | ((unicodevalue >> 12) & 0x3Fu));
    utf8dst[2] = (uint8_t)(0x80u | ((unicodevalue >> 6) & 0x3Fu));
    utf8dst[3] = (uint8_t)(0x80u | (unicodevalue & 0x3Fu));
    return 4;
}

int32_t UnicodeToUTF16(uint32_t unicode, uint16_t *utf16)
{
    if (utf16 == NULL) {
        return -1;
    }
    if (unicode > GBC_UNICODE_MAX) {
        return -1;
    }
    if (unicode >= GBC_SURROGATE_FIRST && unicode <= GBC_SURROGATE_LAST) {
        return -1;
    }

    if (unicode < 0x10000u) {
        utf16[0] = (uint16_t)unicode;
        return 1;
    }

    unicode -= 0x10000u;
    utf16[0] = (uint16_t)(0xD800u | (unicode >> 10));
    utf16[1] = (uint16_t)(0xDC00u | (unicode & 0x3FFu));
    return 2;
}

int32_t UTF16toUnicode(const uint16_t *utf16, uint32_t *outunicode)
{
    uint16_t first;
    uint16_t second;

    if (utf16 == NULL || outunicode == NULL) {
        return -1;
    }

    first = utf16[0];
    if (first < GBC_SURROGATE_FIRST || first > GBC_SURROGATE_LAST) {
        *outunicode = first;
        return 1;
    }
    if (first >= 0xDC00u) {
        /* Lone low surrogate. */
        return -1;
    }

    second = utf16[1];
    if (second < 0xDC00u || second > GBC_SURROGATE_LAST) {
        /* High surrogate not followed by a low surrogate. */
        return -1;
    }

    *outunicode = 0x10000u + (((uint32_t)(first - 0xD800u)) << 10) +
                  (uint32_t)(second - 0xDC00u);
    return 2;
}

int32_t UTF16ToGB2312(uint16_t utf16, uint16_t *gb2312)
{
    if (gb2312 == NULL) {
        return -1;
    }
    return gbc_encode_gb2312(utf16, (uint8_t *)gb2312);
}

int32_t GB2312ToUTF16(uint16_t gb2312, uint16_t *utf16)
{
    if (utf16 == NULL) {
        return -1;
    }
    return gbc_decode_gb2312((const uint8_t *)&gb2312, utf16);
}

int32_t UTF8ToGB2312(uint8_t *utf8, uint32_t *out_utf8_size, uint16_t *gb2312)
{
    uint32_t code_point = 0;
    int32_t consumed;
    int32_t produced;

    if (utf8 == NULL || gb2312 == NULL) {
        return -1;
    }

    consumed = UTF8ToUnicode(utf8, &code_point);
    if (consumed < 0 || code_point > 0xFFFFu) {
        return -1;
    }

    produced = gbc_encode_gb2312((uint16_t)code_point, (uint8_t *)gb2312);
    if (produced < 0) {
        return -1;
    }

    if (out_utf8_size != NULL) {
        *out_utf8_size = (uint32_t)consumed;
    }
    return produced;
}

int32_t GB2312ToUTF8(uint16_t *gb2312, uint32_t *out_gb2312_size, uint8_t *utf8)
{
    uint16_t code_point = 0;
    int32_t consumed;
    int32_t produced;

    if (gb2312 == NULL || utf8 == NULL) {
        return -1;
    }

    consumed = gbc_decode_gb2312((const uint8_t *)gb2312, &code_point);
    if (consumed < 0) {
        return -1;
    }

    produced = UnicodeToUTF8(code_point, utf8);
    if (produced <= 0) {
        return -1;
    }

    if (out_gb2312_size != NULL) {
        *out_gb2312_size = (uint32_t)consumed;
    }
    return produced;
}

int32_t UTF8ToGB2312Str(uint8_t *utf8_str, uint8_t *gb2312_str,
                        size_t gb2312_str_buff_size)
{
    const uint8_t *source = utf8_str;
    uint32_t written = 0;

    if (utf8_str == NULL || gb2312_str == NULL || gb2312_str_buff_size == 0) {
        return 0;
    }

    while (*source != 0) {
        uint32_t code_point = 0;
        uint8_t encoded[2];
        int32_t consumed = UTF8ToUnicode((uint8_t *)source, &code_point);
        int32_t produced;

        if (consumed < 0 || code_point > 0xFFFFu) {
            break;
        }

        produced = gbc_encode_gb2312((uint16_t)code_point, encoded);
        if (produced < 0) {
            break;
        }

        /* Reserve one byte for the terminator. */
        if ((size_t)written + (size_t)produced + 1u > (size_t)gb2312_str_buff_size) {
            break;
        }

        memcpy(gb2312_str + written, encoded, (size_t)produced);
        written += (uint32_t)produced;
        source += consumed;
    }

    gb2312_str[written] = 0;
    return (int32_t)written;
}

int32_t GB2312ToUTF8Str(uint8_t *gb2312_str, uint8_t *utf8_str,
                        size_t utf8_str_buff_size)
{
    const uint8_t *source = gb2312_str;
    uint32_t written = 0;

    if (gb2312_str == NULL || utf8_str == NULL || utf8_str_buff_size == 0) {
        return 0;
    }

    while (*source != 0) {
        uint16_t code_point = 0;
        uint8_t encoded[4];
        int32_t consumed = gbc_decode_gb2312(source, &code_point);
        int32_t produced;

        if (consumed < 0) {
            break;
        }

        produced = UnicodeToUTF8(code_point, encoded);
        if (produced <= 0) {
            break;
        }

        /* Reserve one byte for the terminator. */
        if ((size_t)written + (size_t)produced + 1u > (size_t)utf8_str_buff_size) {
            break;
        }

        memcpy(utf8_str + written, encoded, (size_t)produced);
        written += (uint32_t)produced;
        source += consumed;
    }

    utf8_str[written] = 0;
    return (int32_t)written;
}

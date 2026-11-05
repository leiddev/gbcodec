/*
 * gbcodec - GB2312 <-> Unicode (UTF-8 / UTF-16) conversion library.
 *
 * Copyright (c) 2016-2026 leid
 * SPDX-License-Identifier: ISC
 */
#ifndef UNICODE_UTILITY_H
#define UNICODE_UTILITY_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Decode one UTF-8 encoded code point.
 *
 * @param utf8      Pointer to at least 4 readable bytes (a NUL terminator is
 *                  enough to reject a truncated sequence).
 * @param unicode   Receives the decoded code point. It is left untouched when
 *                  decoding fails.
 * @return Number of bytes consumed (1..4), or -1 on invalid input.
 */
int32_t UTF8ToUnicode(uint8_t *utf8, uint32_t *unicode);

/**
 * Encode one code point as UTF-8.
 *
 * @param unicodevalue Code point in the range U+0000..U+10FFFF, excluding the
 *                     surrogate range U+D800..U+DFFF.
 * @param utf8dst      Buffer of at least 4 bytes.
 * @return Number of bytes written (1..4), or 0 when the code point is invalid.
 */
int32_t UnicodeToUTF8(uint32_t unicodevalue, uint8_t *utf8dst);

/**
 * Encode one code point as UTF-16.
 *
 * @param unicode Code point in the range U+0000..U+10FFFF, excluding the
 *                surrogate range U+D800..U+DFFF.
 * @param utf16   Buffer of at least 2 elements.
 * @return Number of UTF-16 code units written (1 or 2), or -1 on invalid input.
 */
int32_t UnicodeToUTF16(uint32_t unicode, uint16_t *utf16);

/**
 * Decode one UTF-16 code point.
 *
 * @param utf16      Pointer to at least 2 readable elements.
 * @param outunicode Receives the decoded code point. It is left untouched when
 *                   decoding fails.
 * @return Number of code units consumed (1 or 2), or -1 on invalid input
 *         (lone or malformed surrogate pair).
 */
int32_t UTF16toUnicode(const uint16_t *utf16, uint32_t *outunicode);

/**
 * Encode a BMP code point as GB2312.
 *
 * NOTE: @p gb2312 is a two-byte buffer, not a numeric code. The bytes are
 * stored in stream order (lead byte first). Inspect them through a
 * `const uint8_t *` view, e.g. `((const uint8_t *)&buffer)[0]`.
 *
 * @param utf16  Code point (must be in the BMP).
 * @param gb2312 Two-byte output buffer. For ASCII only the first byte is
 *               written; the second byte is left untouched.
 * @return Number of bytes written (1 or 2), or -1 when the code point has no
 *         GB2312 mapping.
 */
int32_t UTF16ToGB2312(uint16_t utf16, uint16_t *gb2312);

/**
 * Decode GB2312 to a BMP code point.
 *
 * NOTE: @p gb2312 is a two-byte buffer in stream order (lead byte first), as
 * produced by UTF16ToGB2312(). Do not pass a numeric literal.
 *
 * @param gb2312 Two-byte GB2312 buffer.
 * @param utf16  Receives the decoded code point.
 * @return Number of bytes consumed (1 or 2), or -1 on invalid input.
 */
int32_t GB2312ToUTF16(uint16_t gb2312, uint16_t *utf16);

/**
 * Convert one UTF-8 character to GB2312.
 *
 * @param utf8           Pointer to at least 4 readable bytes.
 * @param out_utf8_size  Optional. Receives the number of UTF-8 bytes consumed.
 * @param gb2312         Two-byte output buffer in stream order.
 * @return Number of GB2312 bytes written (1 or 2), or -1 on invalid or
 *         unmappable input.
 */
int32_t UTF8ToGB2312(uint8_t *utf8, uint32_t *out_utf8_size, uint16_t *gb2312);

/**
 * Convert one GB2312 character to UTF-8.
 *
 * @param gb2312           Two-byte GB2312 buffer in stream order.
 * @param out_gb2312_size  Optional. Receives the number of GB2312 bytes
 *                         consumed (1 or 2).
 * @param utf8             Buffer of at least 4 bytes.
 * @return Number of UTF-8 bytes written (1..3), or -1 on invalid input.
 */
int32_t GB2312ToUTF8(uint16_t *gb2312, uint32_t *out_gb2312_size, uint8_t *utf8);

/**
 * Convert a NUL-terminated UTF-8 string to a NUL-terminated GB2312 string.
 *
 * Conversion stops at the first invalid or unmappable character, or when the
 * output buffer cannot hold the next character plus the terminator. The output
 * is always terminated when @p gb2312_str_buff_size is greater than zero.
 *
 * @param utf8_str              Source string (NUL-terminated).
 * @param gb2312_str            Output buffer.
 * @param gb2312_str_buff_size  Capacity of the output buffer in bytes.
 * @return Number of bytes written, excluding the terminator.
 */
int32_t UTF8ToGB2312Str(uint8_t *utf8_str, uint8_t *gb2312_str,
                        size_t gb2312_str_buff_size);

/**
 * Convert a NUL-terminated GB2312 string to a NUL-terminated UTF-8 string.
 *
 * Conversion stops at the first invalid character, or when the output buffer
 * cannot hold the next character plus the terminator. The output is always
 * terminated when @p utf8_str_buff_size is greater than zero.
 *
 * @param gb2312_str          Source string (NUL-terminated).
 * @param utf8_str            Output buffer.
 * @param utf8_str_buff_size  Capacity of the output buffer in bytes.
 * @return Number of bytes written, excluding the terminator.
 */
int32_t GB2312ToUTF8Str(uint8_t *gb2312_str, uint8_t *utf8_str,
                        size_t utf8_str_buff_size);

#ifdef __cplusplus
}
#endif

#endif /* UNICODE_UTILITY_H */

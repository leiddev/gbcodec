/*
 * gbcodec - whole-string conversion between UTF-8 and GB2312.
 *
 * Copyright (c) 2016-2026 leid
 * SPDX-License-Identifier: ISC
 */
#ifndef GBCODEC_CONVERT_H
#define GBCODEC_CONVERT_H

#include <gbcodec/export.h>
#include <gbcodec/status.h>

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Conversion between UTF-8 and GB2312 byte strings.
 *
 * Contract for the counted functions below:
 *
 *   - src_length is the exact number of input bytes; the input may contain
 *     embedded NUL bytes and does not need to be NUL-terminated.
 *   - dst_capacity is the total size of dst, in bytes, including the space
 *     reserved for the terminating NUL.
 *   - On success *out_length receives the number of bytes written, excluding
 *     the terminator, and dst is NUL-terminated.
 *   - When dst_capacity is 0, nothing is written to dst and
 *     GBC_E_BUFFER_TOO_SMALL is returned unless the result is empty.
 *   - On any failure dst[0] is set to NUL when dst_capacity is greater than
 *     0 and *out_length (when requested) is left untouched.
 *
 * The _z variants take a NUL-terminated input string and are otherwise
 * identical.
 */

/* Returns the number of bytes the GB2312 result would occupy, excluding NUL. */
GBC_API gbc_status gbc_utf8_to_gb2312_capacity(const uint8_t *src, size_t src_length,
                                               size_t *out_length);

/* Returns the number of bytes the UTF-8 result would occupy, excluding NUL. */
GBC_API gbc_status gbc_gb2312_to_utf8_capacity(const uint8_t *src, size_t src_length,
                                               size_t *out_length);

GBC_API gbc_status gbc_utf8_to_gb2312(const uint8_t *src, size_t src_length, uint8_t *dst,
                                      size_t dst_capacity, size_t *out_length);

GBC_API gbc_status gbc_gb2312_to_utf8(const uint8_t *src, size_t src_length, uint8_t *dst,
                                      size_t dst_capacity, size_t *out_length);

GBC_API gbc_status gbc_utf8_to_gb2312_z(const char *src, char *dst, size_t dst_capacity,
                                        size_t *out_length);

GBC_API gbc_status gbc_gb2312_to_utf8_z(const char *src, char *dst, size_t dst_capacity,
                                        size_t *out_length);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* GBCODEC_CONVERT_H */

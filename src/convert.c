/*
 * gbcodec - whole-string conversion between UTF-8 and GB2312.
 *
 * Copyright (c) 2016-2026 leid
 * SPDX-License-Identifier: ISC
 */
#include <gbcodec/convert.h>

#include <string.h>

#include <gbcodec/gb2312.h>
#include <gbcodec/utf8.h>

/*
 * Converts a counted UTF-8 string to GB2312. When dst is NULL the function
 * only measures the result. Nothing is written to dst when an error occurs
 * after the first pass, because the public entry points measure first.
 */
static gbc_status gbc_write_gb2312(const uint8_t *src, size_t src_length, uint8_t *dst,
                                   size_t *out_written)
{
    size_t offset = 0u;
    size_t written = 0u;

    while (offset < src_length) {
        uint32_t code_point = 0u;
        size_t consumed = 0u;
        uint8_t encoded[2];
        size_t produced;
        gbc_status status;

        status = gbc_utf8_decode(src + offset, src_length - offset, &code_point, &consumed);
        if (status != GBC_OK) {
            return status;
        }

        produced = gbc_gb2312_encode(code_point, encoded);
        if (produced == 0u) {
            return GBC_E_UNMAPPABLE;
        }

        if (dst != NULL) {
            dst[written] = encoded[0];
            if (produced > 1u) {
                dst[written + 1u] = encoded[1];
            }
        }
        written += produced;
        offset += consumed;
    }

    *out_written = written;
    return GBC_OK;
}

/* Converts a counted GB2312 string to UTF-8; see gbc_write_gb2312(). */
static gbc_status gbc_write_utf8(const uint8_t *src, size_t src_length, uint8_t *dst,
                                 size_t *out_written)
{
    size_t offset = 0u;
    size_t written = 0u;

    while (offset < src_length) {
        uint32_t code_point = 0u;
        size_t consumed = 0u;
        uint8_t encoded[4];
        size_t produced;
        size_t index;
        gbc_status status;

        status = gbc_gb2312_decode(src + offset, src_length - offset, &code_point, &consumed);
        if (status != GBC_OK) {
            return status;
        }

        produced = gbc_utf8_encode(code_point, encoded);
        if (produced == 0u) {
            /* Unreachable: every decoded code point is encodable. */
            return GBC_E_UNMAPPABLE;
        }

        if (dst != NULL) {
            for (index = 0u; index < produced; ++index) {
                dst[written + index] = encoded[index];
            }
        }
        written += produced;
        offset += consumed;
    }

    *out_written = written;
    return GBC_OK;
}

typedef gbc_status (*gbc_run_function)(const uint8_t *, size_t, uint8_t *, size_t *);

static gbc_status gbc_convert_counted(const uint8_t *src, size_t src_length, uint8_t *dst,
                                      size_t dst_capacity, size_t *out_length,
                                      gbc_run_function run)
{
    size_t needed = 0u;
    gbc_status status;

    if (src == NULL || dst == NULL) {
        return GBC_E_INVALID_ARGUMENT;
    }

    status = run(src, src_length, NULL, &needed);
    if (status != GBC_OK) {
        if (dst_capacity > 0u) {
            dst[0] = 0u;
        }
        return status;
    }

    /* One byte is always reserved for the terminating NUL. */
    if (needed != 0u && needed + 1u > dst_capacity) {
        if (dst_capacity > 0u) {
            dst[0] = 0u;
        }
        return GBC_E_BUFFER_TOO_SMALL;
    }

    if (dst_capacity > 0u) {
        size_t written = 0u;

        status = run(src, src_length, dst, &written);
        if (status != GBC_OK) {
            dst[0] = 0u;
            return status;
        }
        dst[written] = 0u;
        if (out_length != NULL) {
            *out_length = written;
        }
    } else if (out_length != NULL) {
        *out_length = 0u;
    }

    return GBC_OK;
}

gbc_status gbc_utf8_to_gb2312_capacity(const uint8_t *src, size_t src_length, size_t *out_length)
{
    size_t needed = 0u;
    gbc_status status;

    if (src == NULL || out_length == NULL) {
        return GBC_E_INVALID_ARGUMENT;
    }

    status = gbc_write_gb2312(src, src_length, NULL, &needed);
    if (status != GBC_OK) {
        return status;
    }

    *out_length = needed;
    return GBC_OK;
}

gbc_status gbc_gb2312_to_utf8_capacity(const uint8_t *src, size_t src_length, size_t *out_length)
{
    size_t needed = 0u;
    gbc_status status;

    if (src == NULL || out_length == NULL) {
        return GBC_E_INVALID_ARGUMENT;
    }

    status = gbc_write_utf8(src, src_length, NULL, &needed);
    if (status != GBC_OK) {
        return status;
    }

    *out_length = needed;
    return GBC_OK;
}

gbc_status gbc_utf8_to_gb2312(const uint8_t *src, size_t src_length, uint8_t *dst,
                              size_t dst_capacity, size_t *out_length)
{
    return gbc_convert_counted(src, src_length, dst, dst_capacity, out_length, gbc_write_gb2312);
}

gbc_status gbc_gb2312_to_utf8(const uint8_t *src, size_t src_length, uint8_t *dst,
                              size_t dst_capacity, size_t *out_length)
{
    return gbc_convert_counted(src, src_length, dst, dst_capacity, out_length, gbc_write_utf8);
}

gbc_status gbc_utf8_to_gb2312_z(const char *src, char *dst, size_t dst_capacity,
                                size_t *out_length)
{
    if (src == NULL) {
        return GBC_E_INVALID_ARGUMENT;
    }
    return gbc_utf8_to_gb2312((const uint8_t *)src, strlen(src), (uint8_t *)dst, dst_capacity,
                              out_length);
}

gbc_status gbc_gb2312_to_utf8_z(const char *src, char *dst, size_t dst_capacity,
                                size_t *out_length)
{
    if (src == NULL) {
        return GBC_E_INVALID_ARGUMENT;
    }
    return gbc_gb2312_to_utf8((const uint8_t *)src, strlen(src), (uint8_t *)dst, dst_capacity,
                              out_length);
}

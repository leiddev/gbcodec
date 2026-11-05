/*
 * gbcodec - C++17 wrapper around the C API.
 *
 * Copyright (c) 2016-2026 leid
 * SPDX-License-Identifier: ISC
 *
 * This header is header-only: it needs only the C library. All functions are
 * inline, so including it from several translation units is safe.
 */
#ifndef GBCODEC_GBCODEC_HPP
#define GBCODEC_GBCODEC_HPP

#include <gbcodec/gbcodec.h>

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>

namespace gbcodec {

/* Exception thrown by the throwing conversion helpers. */
class error : public std::runtime_error {
public:
    explicit error(gbc_status status)
        : std::runtime_error(gbc_status_string(status)), status_(status)
    {
    }

    /* The status code that caused the failure. */
    gbc_status status() const noexcept
    {
        return status_;
    }

private:
    gbc_status status_;
};

namespace detail {

using count_function = gbc_status (*)(const std::uint8_t *, std::size_t, std::size_t *);
using convert_function = gbc_status (*)(const std::uint8_t *, std::size_t, std::uint8_t *,
                                        std::size_t, std::size_t *);

/* Measures the result, then converts into a buffer of exactly the right size. */
inline void run(count_function count, convert_function convert, std::string_view input,
                std::string &output)
{
    const auto *const src = reinterpret_cast<const std::uint8_t *>(input.data());
    std::size_t needed = 0;
    gbc_status status = count(src, input.size(), &needed);
    if (status != GBC_OK) {
        throw error(status);
    }

    output.resize(needed + 1);
    std::size_t written = 0;
    status = convert(src, input.size(), reinterpret_cast<std::uint8_t *>(output.data()),
                     output.size(), &written);
    if (status != GBC_OK) {
        output.clear();
        throw error(status);
    }
    output.resize(written);
}

inline bool try_run(count_function count, convert_function convert, std::string_view input,
                    std::string &output, gbc_status *status) noexcept
{
    try {
        run(count, convert, input, output);
    } catch (const error &failure) {
        output.clear();
        if (status != nullptr) {
            *status = failure.status();
        }
        return false;
    } catch (...) {
        output.clear();
        if (status != nullptr) {
            *status = GBC_E_INVALID_ARGUMENT;
        }
        return false;
    }

    if (status != nullptr) {
        *status = GBC_OK;
    }
    return true;
}

} /* namespace detail */

/* Returns the English description of a status code. */
inline const char *status_string(gbc_status status) noexcept
{
    return gbc_status_string(status);
}

/*
 * Converts a UTF-8 string to GB2312.
 *
 * Throws gbcodec::error when the input is malformed or contains a character
 * that GB2312 cannot represent.
 */
inline std::string to_gb2312(std::string_view utf8)
{
    std::string result;
    detail::run(gbc_utf8_to_gb2312_capacity, gbc_utf8_to_gb2312, utf8, result);
    return result;
}

/* Converts a GB2312 string to UTF-8; throws gbcodec::error on failure. */
inline std::string from_gb2312(std::string_view gb2312)
{
    std::string result;
    detail::run(gbc_gb2312_to_utf8_capacity, gbc_gb2312_to_utf8, gb2312, result);
    return result;
}

/*
 * Non-throwing variant of to_gb2312(). Returns false and leaves gb2312 empty
 * on failure; status, when not null, receives the reason.
 */
inline bool try_to_gb2312(std::string_view utf8, std::string &gb2312,
                          gbc_status *status = nullptr) noexcept
{
    return detail::try_run(gbc_utf8_to_gb2312_capacity, gbc_utf8_to_gb2312, utf8, gb2312, status);
}

/*
 * Non-throwing variant of from_gb2312(). Returns false and leaves utf8 empty
 * on failure; status, when not null, receives the reason.
 */
inline bool try_from_gb2312(std::string_view gb2312, std::string &utf8,
                            gbc_status *status = nullptr) noexcept
{
    return detail::try_run(gbc_gb2312_to_utf8_capacity, gbc_gb2312_to_utf8, gb2312, utf8, status);
}

} /* namespace gbcodec */

#endif /* GBCODEC_GBCODEC_HPP */

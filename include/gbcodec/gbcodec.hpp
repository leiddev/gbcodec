/*
 * gbcodec - C++ wrapper around the C API.
 *
 * Copyright (c) 2016-2026 leid
 * SPDX-License-Identifier: ISC
 *
 * This header is header-only: it needs only the C library. All functions are
 * inline, so including it from several translation units is safe.
 *
 * The wrapper requires C++11. When the compiler runs in C++17 mode it uses
 * std::string_view; otherwise it falls back to an equivalent read-only view
 * declared below. In both cases the public parameter type is
 * gbcodec::string_view, so user code does not have to care.
 */
#ifndef GBCODEC_GBCODEC_HPP
#define GBCODEC_GBCODEC_HPP

#include <gbcodec/gbcodec.h>

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>

/*
 * Effective language level. MSVC keeps __cplusplus at 199711L unless
 * /Zc:__cplusplus is passed, so prefer its own _MSVC_LANG macro.
 */
#if defined(_MSVC_LANG)
#define GBCODEC_CPLUSPLUS _MSVC_LANG
#else
#define GBCODEC_CPLUSPLUS __cplusplus
#endif

/* Use std::string_view when the library provides it and we are in C++17. */
#if defined(__has_include)
#if __has_include(<string_view>)
#define GBCODEC_HAVE_STRING_VIEW_HEADER 1
#endif
#endif

#if defined(GBCODEC_HAVE_STRING_VIEW_HEADER) && GBCODEC_CPLUSPLUS >= 201703L
#include <string_view>
#define GBCODEC_USE_STD_STRING_VIEW 1
#endif

namespace gbcodec {

#if defined(GBCODEC_USE_STD_STRING_VIEW)

/* C++17: the standard view, so std::string_view interoperates directly. */
using string_view = std::string_view;

#else

/*
 * C++11 fallback with the subset of std::string_view that the wrapper needs.
 *
 * Like std::string_view it does not own the characters: the referenced buffer
 * must stay alive (and unchanged) for as long as the view is used.
 */
class string_view {
public:
    string_view() noexcept : data_(nullptr), size_(0) {}

    string_view(const char *text) noexcept
        : data_(text),
          size_(text == nullptr ? std::size_t(0) : std::char_traits<char>::length(text))
    {
    }

    string_view(const std::string &text) noexcept : data_(text.data()), size_(text.size()) {}

    string_view(const char *text, std::size_t length) noexcept : data_(text), size_(length) {}

    const char *data() const noexcept
    {
        return data_;
    }

    std::size_t size() const noexcept
    {
        return size_;
    }

    bool empty() const noexcept
    {
        return size_ == 0;
    }

private:
    const char *data_;
    std::size_t size_;
};

#endif /* GBCODEC_USE_STD_STRING_VIEW */

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
inline void run(count_function count, convert_function convert, string_view input,
                std::string &output)
{
    /*
     * A default-constructed std::string_view has a null data() pointer, which
     * the C API rejects. An empty input always yields an empty output, so
     * short-circuit it here to keep "" and std::string_view{} equivalent.
     */
    if (input.empty()) {
        output.clear();
        return;
    }

    const auto *const src = reinterpret_cast<const std::uint8_t *>(input.data());
    std::size_t needed = 0;
    gbc_status status = count(src, input.size(), &needed);
    if (status != GBC_OK) {
        throw error(status);
    }

    output.resize(needed + 1);
    std::size_t written = 0;
    status = convert(src, input.size(), reinterpret_cast<std::uint8_t *>(&output[0]),
                     output.size(), &written);
    if (status != GBC_OK) {
        output.clear();
        throw error(status);
    }
    output.resize(written);
}

inline bool try_run(count_function count, convert_function convert, string_view input,
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
inline std::string to_gb2312(string_view utf8)
{
    std::string result;
    detail::run(gbc_utf8_to_gb2312_capacity, gbc_utf8_to_gb2312, utf8, result);
    return result;
}

/* Converts a GB2312 string to UTF-8; throws gbcodec::error on failure. */
inline std::string from_gb2312(string_view gb2312)
{
    std::string result;
    detail::run(gbc_gb2312_to_utf8_capacity, gbc_gb2312_to_utf8, gb2312, result);
    return result;
}

/*
 * Non-throwing variant of to_gb2312(). Returns false and leaves gb2312 empty
 * on failure; status, when not null, receives the reason.
 */
inline bool try_to_gb2312(string_view utf8, std::string &gb2312,
                          gbc_status *status = nullptr) noexcept
{
    return detail::try_run(gbc_utf8_to_gb2312_capacity, gbc_utf8_to_gb2312, utf8, gb2312, status);
}

/*
 * Non-throwing variant of from_gb2312(). Returns false and leaves utf8 empty
 * on failure; status, when not null, receives the reason.
 */
inline bool try_from_gb2312(string_view gb2312, std::string &utf8,
                            gbc_status *status = nullptr) noexcept
{
    return detail::try_run(gbc_gb2312_to_utf8_capacity, gbc_gb2312_to_utf8, gb2312, utf8, status);
}

} /* namespace gbcodec */

#endif /* GBCODEC_GBCODEC_HPP */

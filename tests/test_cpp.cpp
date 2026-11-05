/*
 * gbcodec - tests for the C++ wrapper.
 *
 * Copyright (c) 2016-2026 leid
 * SPDX-License-Identifier: ISC
 */
#include <gbcodec/string_view.hpp> /* must be self-contained; include it first */
#include <gbcodec/gbcodec.hpp>

#include <cstdio>
#include <cstring>
#include <string>

#if defined(GBCODEC_USE_STD_STRING_VIEW)
#include <string_view>
#include <type_traits>

/* In C++17 mode the wrapper must expose the standard view verbatim. */
static_assert(std::is_same<gbcodec::string_view, std::string_view>::value,
              "gbcodec::string_view must alias std::string_view in C++17");
#endif

namespace {

int g_failures = 0;

void check(bool condition, const char *expression, int line)
{
    if (!condition) {
        ++g_failures;
        std::fprintf(stderr, "line %d: %s failed\n", line, expression);
    }
}

#define CHECK(condition) check((condition), #condition, __LINE__)

const char *const UTF8_TEXT = "\xE4\xB8\xAD\xE6\x96\x87" "abc"; /* 中文abc */
const char *const GB2312_TEXT = "\xD6\xD0\xCE\xC4" "abc";

void test_roundtrip()
{
    const std::string utf8(UTF8_TEXT);
    const std::string gb2312 = gbcodec::to_gb2312(utf8);

    CHECK(gb2312.size() == 7u);
    CHECK(gb2312.compare(0u, 4u, std::string(GB2312_TEXT, 4u)) == 0);
    CHECK(gbcodec::from_gb2312(gb2312) == utf8);
}

void test_try_variants()
{
    gbc_status status = GBC_OK;
    std::string output;

    CHECK(gbcodec::try_to_gb2312("abc", output, &status));
    CHECK(output == "abc");
    CHECK(status == GBC_OK);

    output = "unchanged";
    CHECK(!gbcodec::try_to_gb2312("\xE2\x82\xAC", output, &status)); /* U+20AC */
    CHECK(status == GBC_E_UNMAPPABLE);
    CHECK(output.empty());

    CHECK(gbcodec::try_from_gb2312(std::string("\xD6\xD0", 2u), output, &status));
    CHECK(status == GBC_OK);
    CHECK(output == std::string("\xE4\xB8\xAD", 3u));

    /* The status argument is optional. */
    CHECK(gbcodec::try_to_gb2312("xyz", output));
}

void test_exception()
{
    bool caught = false;

    try {
        static_cast<void>(gbcodec::to_gb2312("\xFF"));
    } catch (const gbcodec::error &failure) {
        caught = true;
        CHECK(failure.status() == GBC_E_INVALID_SEQUENCE);
        CHECK(std::strcmp(failure.what(), gbc_status_string(GBC_E_INVALID_SEQUENCE)) == 0);
    }
    CHECK(caught);

    CHECK(std::strcmp(gbcodec::status_string(GBC_E_TRUNCATED), "truncated byte sequence") == 0);
}

void test_string_view()
{
    const gbcodec::string_view empty;
    const gbcodec::string_view literal("abc");
    const std::string text("hello");
    const gbcodec::string_view from_string(text);
    const gbcodec::string_view slice(literal.data(), 2u);

    CHECK(empty.empty());
    CHECK(empty.size() == 0u);
    CHECK(empty.data() == nullptr);

    CHECK(!literal.empty());
    CHECK(literal.size() == 3u);

    CHECK(from_string.data() == text.data());
    CHECK(from_string.size() == text.size());

    CHECK(slice.size() == 2u);
    CHECK(slice.data() == literal.data());
}

void test_empty_strings()
{
    CHECK(gbcodec::to_gb2312("").empty());
    CHECK(gbcodec::from_gb2312("").empty());
}

void test_lossy_conversion()
{
    /* 中文😀abc */
    const std::string emoji_input("\xE4\xB8\xAD\xE6\x96\x87\xF0\x9F\x98\x80" "abc");
    std::size_t replaced = 0u;

    /* "中文😀abc" -> "中文?abc" */
    const std::string lossy = gbcodec::to_gb2312(emoji_input, U'?', replaced);
    CHECK(lossy.size() == 8u);
    CHECK(replaced == 1u);
    CHECK(lossy.compare(0u, 4u, std::string(GB2312_TEXT, 4u)) == 0);
    CHECK(lossy[4] == '?');

    /* A zero replacement deletes the character; the count still reports it. */
    replaced = 0u;
    const std::string deleted = gbcodec::to_gb2312(emoji_input, 0u, replaced);
    CHECK(deleted == std::string(GB2312_TEXT));
    CHECK(replaced == 1u);

    /* Unassigned GB2312 pairs are substituted on the way back. */
    replaced = 0u;
    const std::string decoded =
        gbcodec::from_gb2312(std::string("\xD6\xD0\xAA\xA1", 4u), U'\uFFFD', replaced);
    CHECK(replaced == 1u);
    CHECK(decoded == std::string("\xE4\xB8\xAD\xEF\xBF\xBD", 6u));

    /* An unusable replacement is an argument error, even for empty input. */
    bool caught = false;
    try {
        static_cast<void>(gbcodec::to_gb2312("", U'\u20AC'));
    } catch (const gbcodec::error &failure) {
        caught = true;
        CHECK(failure.status() == GBC_E_INVALID_ARGUMENT);
    }
    CHECK(caught);

    /* The non-throwing variants behave the same way. */
    std::string output;
    gbc_status status = GBC_OK;
    std::size_t count = 0u;
    CHECK(gbcodec::try_to_gb2312("\xF0\x9F\x98\x80", U'?', output, &count, &status));
    CHECK(status == GBC_OK);
    CHECK(output == "?");
    CHECK(count == 1u);

    output = "stale";
    count = 99u;
    CHECK(!gbcodec::try_to_gb2312("\xF0\x9F\x98\x80", U'\u20AC', output, &count, &status));
    CHECK(status == GBC_E_INVALID_ARGUMENT);
    CHECK(output.empty());
    CHECK(count == 99u);

    /* Malformed input still fails. */
    CHECK(!gbcodec::try_to_gb2312("\xFF", U'?', output, &count, &status));
    CHECK(status == GBC_E_INVALID_SEQUENCE);

    /* A default-constructed view with a valid replacement is an empty result. */
    CHECK(gbcodec::to_gb2312(gbcodec::string_view(), U'?').empty());
}

/*
 * A default-constructed std::string_view is empty but has a null data()
 * pointer; it must behave exactly like "" rather than being rejected by the
 * C API.
 */
void test_default_constructed_view()
{
    const gbcodec::string_view empty_view;
    std::string output = "stale";
    gbc_status status = GBC_E_TRUNCATED;

    CHECK(gbcodec::to_gb2312(empty_view).empty());
    CHECK(gbcodec::from_gb2312(empty_view).empty());

    CHECK(gbcodec::try_to_gb2312(empty_view, output, &status));
    CHECK(output.empty());
    CHECK(status == GBC_OK);

    output = "stale";
    status = GBC_E_TRUNCATED;
    CHECK(gbcodec::try_from_gb2312(empty_view, output, &status));
    CHECK(output.empty());
    CHECK(status == GBC_OK);
}

} /* namespace */

int main()
{
    test_roundtrip();
    test_try_variants();
    test_exception();
    test_string_view();
    test_empty_strings();
    test_default_constructed_view();
    test_lossy_conversion();

    std::printf("C++ wrapper: %s\n", g_failures == 0 ? "all checks passed" : "checks failed");
    return g_failures == 0 ? 0 : 1;
}

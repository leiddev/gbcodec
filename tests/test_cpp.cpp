/*
 * gbcodec - tests for the C++17 wrapper.
 *
 * Copyright (c) 2016-2026 leid
 * SPDX-License-Identifier: ISC
 */
#include <gbcodec/gbcodec.hpp>

#include <cstdio>
#include <cstring>
#include <string>

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

void test_empty_strings()
{
    CHECK(gbcodec::to_gb2312("").empty());
    CHECK(gbcodec::from_gb2312("").empty());
}

} /* namespace */

int main()
{
    test_roundtrip();
    test_try_variants();
    test_exception();
    test_empty_strings();

    std::printf("C++ wrapper: %s\n", g_failures == 0 ? "all checks passed" : "checks failed");
    return g_failures == 0 ? 0 : 1;
}

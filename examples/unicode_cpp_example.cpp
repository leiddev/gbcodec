/*
 * gbcodec - basic C++ usage example.
 *
 * Copyright (c) 2016-2026 leid
 * SPDX-License-Identifier: ISC
 */
#include <cstdio>
#include <string>

#include "unicode_utility_cpp.h"

static void print_hex(const char *label, const std::string &data)
{
    std::printf("%s:", label);
    for (std::string::size_type index = 0; index < data.size(); ++index) {
        std::printf(" %02X", static_cast<unsigned char>(data[index]));
    }
    std::printf("\n");
}

int main()
{
    /* "中文abc" in UTF-8, written as bytes to stay independent of the
     * source file encoding. */
    static const char utf8_text[] = "\xE4\xB8\xAD\xE6\x96\x87" "abc";
    std::string input(utf8_text);

    std::string gb2312 = UnicodeUtility::UTF8ToGB2312(input);
    if (gb2312.empty()) {
        std::fprintf(stderr, "UTF-8 to GB2312 conversion failed\n");
        return 1;
    }
    print_hex("UTF-8  ", input);
    print_hex("GB2312 ", gb2312);

    std::string back = UnicodeUtility::GB2312ToUTF8(gb2312);
    print_hex("UTF-8  ", back);

    return back == input ? 0 : 1;
}

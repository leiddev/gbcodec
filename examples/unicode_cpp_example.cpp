/*
 * gbcodec - basic C++ example.
 *
 * Copyright (c) 2016-2026 leid
 * SPDX-License-Identifier: ISC
 */
#include <gbcodec/gbcodec.hpp>

#include <cstdio>
#include <iostream>
#include <string>
#include <string_view>

int main()
{
    const std::string_view utf8_text = "\xE4\xB8\xAD\xE6\x96\x87" "abc"; /* 中文abc */

    try {
        const std::string gb2312 = gbcodec::to_gb2312(utf8_text);
        const std::string decoded = gbcodec::from_gb2312(gb2312);

        std::printf("GB2312: %zu bytes\n", gb2312.size());
        std::printf("UTF-8:  %s\n", decoded.c_str());
        return decoded == utf8_text ? 0 : 1;
    } catch (const gbcodec::error &failure) {
        std::cerr << "conversion failed: " << failure.what() << '\n';
        return 1;
    }
}

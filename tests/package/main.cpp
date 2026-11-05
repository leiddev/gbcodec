/*
 * gbcodec - installed-package smoke test (C++).
 *
 * Copyright (c) 2016-2026 leid
 * SPDX-License-Identifier: ISC
 */
#include <gbcodec/gbcodec.hpp>

#include <cstdio>
#include <string>

int main()
{
    const std::string utf8_text = "\xE4\xB8\xAD\xE6\x96\x87" "abc";

    try {
        const std::string gb2312 = gbcodec::to_gb2312(utf8_text);
        const std::string decoded = gbcodec::from_gb2312(gb2312);
        std::printf("gbcodec %s (C++): %zu bytes\n", GBCODEC_VERSION_STRING, gb2312.size());
        return decoded == utf8_text ? 0 : 1;
    } catch (const gbcodec::error &failure) {
        std::fprintf(stderr, "conversion failed: %s\n", failure.what());
        return 1;
    }
}

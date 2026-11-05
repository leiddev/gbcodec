# 变更日志

本项目遵循 [语义化版本](https://semver.org/lang/zh-CN/) 与
[Keep a Changelog](https://keepachangelog.com/zh-CN/1.1.0/)。

## [Unreleased]

### 修复

- C++ 封装：默认构造的 `std::string_view`（`data()` 为 `nullptr`）现在与空字符串
  行为一致，直接返回空结果，不再被 C 层的空指针检查拒绝。

## [1.0.0] - 2026-11-05

第一个正式版本。相对历史实现（0.0.0 基线）而言，这一版重建了码表、修复了内存
安全问题，并把整套接口重新设计为 `gbcodec`。

### 破坏性变更

- 公开 API 整体重写为 `gbcodec`：头文件为 `<gbcodec/...>`，C 函数前缀 `gbc_`，
  宏前缀 `GBC_`，C++ 命名空间为 `gbcodec`。历史接口（`inc/unicode_utility.h`、
  `inc/unicode_utility_cpp.h` 与 `UnicodeUtility` 命名空间）已删除。
- 所有函数改为返回 `gbc_status` 错误码，并显式接收长度参数；单字符接口统一为
  `gbc_*_decode(src, src_length, out_code_point, out_consumed)` 与
  `gbc_*_encode(code_point, out)` 两种形状。
- GB2312 缓冲区不再用 `uint16_t` 承载，改为字节流（`uint8_t` / `char`），
  与主机字节序无关。

历史接口与新接口的对应关系：

| 历史接口 | 现在的接口 |
| --- | --- |
| `UTF8ToUnicode` | `gbc_utf8_decode` |
| `UnicodeToUTF8` | `gbc_utf8_encode` |
| `UnicodeToUTF16` | `gbc_utf16_encode` |
| `UTF16toUnicode` | `gbc_utf16_decode` |
| `UTF16ToGB2312` | `gbc_gb2312_encode`（先用 `gbc_utf16_decode` 得到码点） |
| `GB2312ToUTF16` | `gbc_gb2312_decode` + `gbc_utf16_encode` |
| `UTF8ToGB2312` / `UTF8ToGB2312Str` | `gbc_utf8_to_gb2312` / `gbc_utf8_to_gb2312_z` |
| `GB2312ToUTF8` / `GB2312ToUTF8Str` | `gbc_gb2312_to_utf8` / `gbc_gb2312_to_utf8_z` |

### 新增

- 项目骨架：ISC 许可证、`.gitignore` / `.gitattributes` / `.editorconfig`、
  clang-format / clang-tidy 配置。
- 现代 CMake 构建系统，库目标命名为 `gbcodec`，支持静态库与共享库，并提供
  `CMakePresets.json`。
- GitHub Actions CI：Windows（x64 / x86，MSVC）与 Linux（x64 / x86，GCC），
  另含共享库构建、ASan + UBSan 以及码表一致性检查。
- C++17 封装 `<gbcodec/gbcodec.hpp>`：`gbcodec::to_gb2312` / `from_gb2312`（抛出
  `gbcodec::error`）与 `try_to_gb2312` / `try_from_gb2312`（返回 `bool`）。
- `gbc_status_string`：把错误码转换为固定的英文描述。
- `gbc_utf8_to_gb2312_capacity` / `gbc_gb2312_to_utf8_capacity`：在写入前查询
  结果长度，便于一次性分配缓冲区。
- 共享库导出控制（`<gbcodec/export.h>`、`GBC_API`、`-fvisibility=hidden`）与
  `version.h` 版本宏。
- 单元测试：五个 C 测试程序加一个 C++ 测试程序，覆盖非法序列、截断、
  缓冲区边界矩阵、内嵌 NUL、容量查询、全码表往返等场景。
- 安装规则与包配置：支持 `find_package(gbcodec CONFIG)`（导出
  `gbcodec::gbcodec` 与 `gbcodec::gbcodec_cpp`），并提供 `gbcodec.pc`
  pkg-config 元数据。
- `tests/package` 消费者工程，用于验证安装后的头文件、库与包配置。
- 文档：`docs/naming.md`（命名约定）、`docs/compatibility.md`（GB2312 与
  GBK / CP936 的差异）、`CONTRIBUTING.md`、`SECURITY.md`。

### 变更

- 构建目标由 `unicode_utility` 统一为 `gbcodec`。
- 码表改为由 `tools/gen_gb2312_table.py` 生成（数据源为 Python 的 `gb2312`
  编解码器），并纳入 CI 校验，避免手改产生漂移。
- 码表条目由 7461 条修正为 GB2312-80 的 7445 条（6763 个汉字 + 682 个符号）。
- GB2312 → Unicode 改为 O(1) 直接索引；Unicode → GB2312 改为 O(log n) 二分查找，
  整串转换不再是 O(n·m)。
- 公共头不再暴露内部码表：删除 `inc/utf16_to_gb2312_table.h` 与 `MAX_UTF16_SIZE`。
- `UnicodeToUTF8` 的首个参数由 `unsigned long` 改为 `uint32_t`，消除 LP64 / LLP64
  差异；返回值统一为 `int32_t`。
- `UTF16toUnicode` 的入参改为 `const uint16_t *`，输出改为 `uint32_t *`。
- 字符串转换接口的缓冲区容量参数改为 `size_t`。
- `GBCODEC_BUILD_CXX` 选项：作为子项目引用时默认不再强制启用 C++ 编译器。
- 可执行文件与 Windows DLL 统一输出到 `build/bin/<配置>/`。

### 修复

- 修复 `UTF8ToGB2312Str` 缓冲区容量判断漏算当前字符长度导致的堆越界写；
  现在始终为终止符预留一个字节，缓冲区不足时安全失败。
- 修复 `UTF8ToUnicode` / `UTF16toUnicode` / `UTF16ToGB2312` / `GB2312ToUTF16` /
  `GB2312ToUTF8` 在参数为 `NULL` 时的解引用。
- 修复 `UTF16ToGB2312` / `GB2312ToUTF16` 只写 16 位缓冲区低字节、导致调用方
  高字节残留脏数据的问题。
- 修复 ASCII 判定阈值（原为 `0xFF` / `0xA1A0`），改用 `0x80`；`U+00A4` 等 19 个
  字符现在能正确映射为两字节 GB2312 码。
- 按 RFC 3629 严格校验 UTF-8：拒绝超长编码、代理项、超出 `U+10FFFF` 的码点以及
  非法首字节；编码端同样拒绝代理项与越界码点。
- 修复 `UnicodeToUTF8` 依赖小端内存布局的取位方式，改为显式移位，大端平台结果
  同样正确。
- 修复 `UnicodeToUTF16` 的码点上限（原为 `0xEFFFF`）与代理项校验。
- 修复空字符串或首字符非法时输出缓冲区未写入终止符的问题。
- 修复 `UTF16toUnicode` 未校验低代理项、可能返回错误码点的问题。
- 修复 GB2312 解码未校验首尾字节范围（首 `0xA1`–`0xF7`，尾 `0xA1`–`0xFE`）。

### 移除

- 移除不属于 GB2312 的映射：GBK 新增的欧元符号 `U+20AC`、小型罗马数字
  `U+2170..U+2179`，以及旧表中映射到私用区的 `0xD7FA..0xD7FE`。
- 删除历史实现文件 `inc/unicode_utility.h`、`inc/unicode_utility_cpp.h`、
  `src/unicode_utility.c`、`src/unicode_utility_cpp.cpp`。

## [0.0.0] - 2026-11-05

### 初始

- 导入历史实现作为基线提交，保留原始文件与行为以便对照。

[Unreleased]: https://github.com/leiddev/gbcodec/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/leiddev/gbcodec/releases/tag/v1.0.0

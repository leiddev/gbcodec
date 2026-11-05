# 变更日志

本项目遵循 [语义化版本](https://semver.org/lang/zh-CN/) 与
[Keep a Changelog](https://keepachangelog.com/zh-CN/1.1.0/)。

## [Unreleased]

### 变更

- 码表改为由 `tools/gen_gb2312_table.py` 生成（数据源为 Python 的 `gb2312`
  编解码器），并纳入 CI 校验，避免手改产生漂移。
- 码表条目由 7461 条修正为 GB2312-80 的 7445 条（6763 汉字 + 682 符号）。
- GB2312 → Unicode 改为 O(1) 直接索引；Unicode → GB2312 改为 O(log n) 二分查找，
  整串转换不再是 O(n·m)。
- 公共头不再暴露内部码表：删除 `inc/utf16_to_gb2312_table.h` 与 `MAX_UTF16_SIZE`。

### 移除

- 移除不属于 GB2312 的映射：GBK 新增的欧元符号 `U+20AC`、小型罗马数字
  `U+2170..U+2179`，以及旧表中映射到私用区的 `0xD7FA..0xD7FE`。

### 修复

- 修复 `UTF8ToGB2312Str` 缓冲区容量判断漏算当前字符长度导致的堆越界写；
  现在始终为终止符预留一个字节，缓冲区不足时安全截断。
- 修复 `UTF8ToUnicode` / `UTF16toUnicode` / `UTF16ToGB2312` / `GB2312ToUTF16` /
  `GB2312ToUTF8` 在参数为 `NULL` 时的解引用。
- 修复 `UTF16ToGB2312` / `GB2312ToUTF16` 只写 16 位缓冲区低字节、导致调用方
  高字节残留脏数据的问题；GB2312 相关接口统一按"报文字节流"处理，高字节问题
  与字节序无关。
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

### 变更

- `UnicodeToUTF8` 的首个参数由 `unsigned long` 改为 `uint32_t`，消除 LP64/LLP64
  差异；返回值统一为 `int32_t`。
- `UTF16toUnicode` 的入参改为 `const uint16_t *`，输出改为 `uint32_t *`。
- 字符串转换接口的缓冲区容量参数改为 `size_t`。
- `GBCODEC_BUILD_CXX` 选项：作为子项目引用时默认不再强制启用 C++ 编译器。

### 新增

- 项目骨架：ISC 许可证、`.gitignore` / `.gitattributes` / `.editorconfig`、
  clang-format / clang-tidy 配置。
- 现代 CMake 构建系统，库目标命名为 `gbcodec`，支持静态库与共享库。
- GitHub Actions CI：Windows（x64/x86，MSVC）与 Linux（x64/x86，GCC），
  另含 ASan + UBSan 任务。
- `CMakePresets.json` 与冒烟测试。

### 变更

- 构建目标由 `unicode_utility` 统一为 `gbcodec`。

## [0.0.0] - 2026-11-05

### 初始

- 导入历史实现作为基线提交，保留原始文件与行为以便对照。

# gbcodec

[![CI](https://github.com/leiddev/gbcodec/actions/workflows/ci.yml/badge.svg)](https://github.com/leiddev/gbcodec/actions/workflows/ci.yml)
[![License: ISC](https://img.shields.io/badge/license-ISC-blue.svg)](LICENSE)

`gbcodec` 是一个轻量的 C / C++ 库，用于 **GB2312** 与 **Unicode（UTF-8 / UTF-16）**
之间的相互转换。它不依赖任何第三方库，也不依赖操作系统的字符集 API，因此可以在
Windows 与 Linux 上得到完全一致的结果。

## 特性

- 纯 C99 实现，可选 C++17 封装，无第三方依赖。
- 转换结果只取决于码表，与操作系统区域设置（locale / code page）无关。
- 显式长度与容量参数，不使用全局状态，线程安全。
- 同时提供单字符（码点级）与整串（缓冲区级）转换接口。
- 完整的错误码：非法序列、截断、无法映射、缓冲区不足可分别判断。
- 内置 O(1) / O(log n) 查表，不使用线性扫描。
- 在 Windows（MSVC、MinGW）与 Linux（GCC、Clang）的 x86 / x64 上持续集成。

## 支持的平台

| 平台 | 架构 | 工具链 | 状态 |
| --- | --- | --- | --- |
| Windows | x64 | MSVC 2019+ | CI 覆盖 |
| Windows | x86 | MSVC 2019+ | CI 覆盖 |
| Linux | x64 | GCC 9+ / Clang 10+ | CI 覆盖 |
| Linux | x86 | GCC 9+（`-m32`） | CI 覆盖 |

> 目前只覆盖 GB2312（6763 个汉字 + 682 个符号）。GBK / GB18030 不在 1.0 的范围内，
> 详见 [docs/compatibility.md](docs/compatibility.md)。

## 项目状态

当前仓库正处于从历史实现向 1.0 重构的过渡期：

- `main` 分支在 1.0 发布前不保证 API 与 ABI 稳定；
- 历史实现（`inc/unicode_utility.h`）暂时保留，1.0 将整体替换为
  `<gbcodec/gbcodec.h>`；
- 迁移说明见 [CHANGELOG.md](CHANGELOG.md)。

## 构建

需要 CMake 3.15 及以上版本，以及一个支持 C99 / C++17 的编译器。

```bash
# 配置 + 构建 + 测试
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure

# 安装
cmake --install build --prefix /your/prefix
```

Windows 上使用 Visual Studio 生成器时可以指定架构：

```bat
cmake -S . -B build -A x64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

也可以直接使用预设：

```bash
cmake --preset linux-x64        # 或 windows-x64 / windows-x86 / linux-x86
cmake --build --preset linux-x64
ctest --preset linux-x64
```

### 构建选项

| 选项 | 默认值 | 说明 |
| --- | --- | --- |
| `BUILD_SHARED_LIBS` | `OFF` | 构建共享库 |
| `GBCODEC_BUILD_CXX` | 顶层项目时 `ON` | 构建 C++ 封装；作为子项目时默认关闭，避免强制依赖 C++ 编译器 |
| `GBCODEC_BUILD_EXAMPLES` | 顶层项目时 `ON` | 构建示例程序 |
| `GBCODEC_BUILD_TESTS` | 顶层项目时 `ON` | 构建单元测试 |
| `GBCODEC_WARNINGS_AS_ERRORS` | `OFF` | 把编译告警当作错误 |

## 目录结构

```text
.
├─ include/        公开头文件（1.0 起）
├─ inc/            历史头文件（过渡期保留）
├─ src/            实现与码表
├─ examples/       示例程序
├─ tests/          单元测试
├─ tools/          码表生成脚本
├─ docs/           设计、命名与兼容性文档
└─ .github/        CI 与协作模板
```

## 路线图

- [x] 建立仓库、导入基线、搭建 CI（Windows / Linux，x86 / x64）
- [ ] 补齐测试并修复内存安全与正确性缺陷
- [ ] 重建码表，改为 O(1) / O(log n) 查找
- [ ] 重新设计并重命名公开 API（`gbcodec` / `gbc_` 前缀 / C++17 封装）
- [ ] 打包、文档与 1.0.0 发布

## 参与贡献

请先阅读 [CONTRIBUTING.md](CONTRIBUTING.md)。提交缺陷时请附上平台、编译器版本与
最小复现用例。

## 许可证

[ISC](LICENSE)，Copyright (c) 2016-2026 leid。

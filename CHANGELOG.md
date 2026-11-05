# 变更日志

本项目遵循 [语义化版本](https://semver.org/lang/zh-CN/) 与
[Keep a Changelog](https://keepachangelog.com/zh-CN/1.1.0/)。

## [Unreleased]

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

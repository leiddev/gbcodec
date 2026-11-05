# 参与贡献

感谢你愿意为 `gbcodec` 做出贡献。本文档说明构建方式、代码规范与提交流程。

## 开发环境

- CMake 3.15 及以上；
- C 编译器：MSVC 2019+、GCC 9+ 或 Clang 10+；
- C++ 编译器（可选，用于 C++ 封装与测试）：C++11 及以上；
- Python 3.8+（仅在修改码表时需要）。

## 构建与测试

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DGBCODEC_WARNINGS_AS_ERRORS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Windows 上指定架构：

```bat
cmake -S . -B build -A x64 -DGBCODEC_WARNINGS_AS_ERRORS=ON
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

也可以使用预设（`CMakePresets.json`）：`windows-x64`、`windows-x86`、
`linux-x64`、`linux-x86`。

提交前请至少验证一种 64 位与一种 32 位配置。32 位 Linux 需要
`gcc-multilib` / `g++-multilib`。

## 代码风格

- C 语言标准为 C99（`-std=c99`、禁用扩展）；C++ 封装支持 C++11 及以上，
  CI 会分别以 C++11 与 C++17 两种模式编译测试（`test_cpp11` / `test_cpp`）。
- 缩进 4 空格，不使用 Tab；行宽 100 列。配置见 `.clang-format`，可直接运行
  `clang-format -i` 格式化改动过的文件。
- 变量、函数、文件名使用 `snake_case`，宏使用大写下划线。完整规则见
  [docs/naming.md](docs/naming.md)。
- 所有源码文件都带 ISC 许可头：

  ```c
  /*
   * gbcodec - <一句话说明>.
   *
   * Copyright (c) 2016-2026 leid
   * SPDX-License-Identifier: ISC
   */
  ```

- 代码注释使用英文；面向使用者的文档（README、CHANGELOG、docs）使用中文。
- 不允许出现编译告警：CI 使用 `GBCODEC_WARNINGS_AS_ERRORS=ON` 构建，
  GCC/Clang 开启 `-Wall -Wextra -Wpedantic -Wshadow -Wconversion
  -Wsign-conversion`，MSVC 开启 `/W4 /permissive- /utf-8`。

## 修改码表

`src/detail/gb2312_table.c` 是生成文件，**不要手工编辑**：

```bash
python3 tools/gen_gb2312_table.py            # 重新生成
python3 tools/gen_gb2312_table.py --check    # 仅校验，CI 使用的就是这条命令
```

如果确实需要调整映射（例如新增或移除某个字符），请修改生成脚本，然后重新生成
并说明依据（标准号、出处）。

## 修改公开 API

- 任何新增的 C 函数都必须使用 `gbc_` 前缀，并在 `include/gbcodec/` 下声明，
  在 `src/` 下实现。
- 返回值只能是 `gbc_status`、`size_t` 或 `const char *` 三种形状之一。
- 新增接口必须带单元测试（`tests/`），失败路径也要覆盖：`NULL` 参数、
  截断输入、缓冲区不足、无法映射。
- 破坏性改动需要同步更新 `CHANGELOG.md` 的「破坏性变更」小节与
  `docs/compatibility.md`。

## 测试

- 测试框架是自带的 `tests/test_common.h`，不引入第三方依赖。
- 每个测试程序必须能独立运行并返回非零退出码表示失败。
- 涉及内存安全的改动请在本地再跑一次 AddressSanitizer：

  ```bash
  cmake -S . -B build-asan -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_C_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer" \
    -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer" \
    -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined"
  cmake --build build-asan --parallel
  ctest --test-dir build-asan --output-on-failure
  ```

## 提交信息

采用 Conventional Commits 风格，第一行不超过 72 个字符：

```text
<type>(<scope>): <summary>

<body>
```

常用 `type`：`feat`、`fix`、`perf`、`refactor`、`docs`、`test`、`build`、`ci`、
`chore`。破坏性变更在类型后加 `!`，并在正文中说明迁移方式。

## 提交前检查清单

- [ ] `cmake --build` 无告警（建议开启 `GBCODEC_WARNINGS_AS_ERRORS`）
- [ ] `ctest` 全部通过
- [ ] 新增行为有对应测试
- [ ] `python3 tools/gen_gb2312_table.py --check` 通过
- [ ] 更新了 `CHANGELOG.md`（以及需要时的 README / docs）
- [ ] 文件使用 LF 换行（`.gitattributes` 已处理，注意不要引入 CRLF）

## 安全问题

请勿通过公开的 issue 报告安全漏洞，流程见 [SECURITY.md](SECURITY.md)。

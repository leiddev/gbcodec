# 字符集兼容性说明

`gbcodec` 只实现 **GB2312-80**（中国国家标准简体中文字符集）与 Unicode 之间的
转换。本文说明它的边界，以及和 GBK / CP936 / GB18030 相处时需要注意的地方。

## GB2312 覆盖范围

- 编码空间为两个 94 行的区：首字节 `0xA1`–`0xF7`（87 个区），尾字节
  `0xA1`–`0xFE`（94 个位）。
- 共收录 **7445** 个字符：6763 个汉字 + 682 个符号（标点、希腊字母、西里尔
  字母、日文假名、制表符等）。
- ASCII（`0x00`–`0x7F`）在 GB2312 与 UTF-8 中完全一致，`gbcodec` 直接原样
  传递，不做任何改写。

库内码表由 `tools/gen_gb2312_table.py` 生成，数据源是语言运行时的 GB2312
解码结果；CI 会在每次提交时用 `--check` 重新生成并比对，保证码表与生成器一致。

## 与 GBK / CP936 的差异

GBK 是 GB2312 的超集：保留全部 GB2312 区位，并增加：

- 尾字节 `0x40`–`0xA0` 的第二字节空间；
- 首字节 `0x80`–`0xA0` 与 `0xF8`–`0xFE` 的扩展区；
- GB2312 未收录的繁体字、生僻字与部分符号，例如欧元符号 `U+20AC`
  （GBK `0xA2E3`）、小型罗马数字 `U+2170..U+2179`（GBK `0xA2A1` 起）、
  私用区字符（GBK `0xD7FA..0xD7FE` 等）。

`gbcodec` 明确**不包含**这些扩展：

- 解码时，落在 GB2312 范围之外的首字节/尾字节返回 `GBC_E_INVALID_SEQUENCE`；
- 落在 GB2312 范围之内、但没有分配字符的码位返回 `GBC_E_UNMAPPABLE`；
- 编码时，GBK 独有的字符返回长度 0（单字符接口）或 `GBC_E_UNMAPPABLE`
  （整串接口）。

Windows 的代码页 936（CP936）与 GBK 基本等价，因此从 CP936 数据迁移到本库时，
应先用 GBK 解码器把数据转成 Unicode，再用 `gbcodec` 输出 GB2312，并处理无法
映射的字符（见下文）。

历史实现的码表中包含 `U+20AC` 等 GBK 字符，这些条目在 1.0 中已被移除，
详见 [CHANGELOG](../CHANGELOG.md)。

## 遇到无法映射的字符怎么办

本节只讨论 `GBC_E_UNMAPPABLE`：输入本身完全合法，只是目标字符集没有对应字符
（例如 `€` `U+20AC`、`😀` `U+1F600`）。输入损坏（`GBC_E_INVALID_SEQUENCE`）或
被截断（`GBC_E_TRUNCATED`）不属于「无法映射」，也不应该通过替换字符来掩盖，
三种错误码的判定边界见 [errors.md](errors.md)。

`gbcodec` 不做任何隐式替换（不会把无法映射的字符换成 `?` 或全角问号），原因
是隐式替换会静默改变数据。调用方需要显式决定策略：

- **报错**：直接使用 `gbc_utf8_to_gb2312` / `gbc_utf8_to_gb2312_z`，检查返回的
  `GBC_E_UNMAPPABLE`（同一次调用也可能返回 `GBC_E_INVALID_SEQUENCE` 等，需分别处理）；
- **替换**：自行遍历输入，用 `gbc_utf8_decode` + `gbc_gb2312_encode` 逐字符
  转换，`gbc_gb2312_encode` 返回 0 时写入替代字符：`?`（`0x3F`，1 字节）或
  全角问号 `A3 BF`（`U+FF1F`，2 字节）；
- **跳过**：逐字符转换时直接丢弃无法映射的码点。

注意「替换」与「跳过」只适用于 `gbc_utf8_decode` **成功**返回的字符；解码失败的
字节必须单独处理，否则会把损坏的数据当成「无法映射」悄悄吞掉。

「替换」策略的完整写法：

```c
#include <gbcodec/gbcodec.h>

/* 把 UTF-8 转成 GB2312，无法映射的字符替换为全角问号 A3 BF。
 * 输入损坏或截断时直接返回对应错误，不做替换。 */
static gbc_status convert_with_replacement(const uint8_t *src, size_t src_length,
                                           uint8_t *dst, size_t dst_capacity,
                                           size_t *out_length)
{
    size_t offset = 0;
    size_t written = 0;

    while (offset < src_length) {
        uint32_t code_point = 0;
        size_t consumed = 0;
        uint8_t encoded[2] = {0};
        size_t produced;
        gbc_status status = gbc_utf8_decode(src + offset, src_length - offset,
                                            &code_point, &consumed);
        if (status != GBC_OK) {
            return status; /* GBC_E_INVALID_SEQUENCE 或 GBC_E_TRUNCATED */
        }

        produced = gbc_gb2312_encode(code_point, encoded);
        if (produced == 0) {
            encoded[0] = 0xA3; /* 全角问号 U+FF1F */
            encoded[1] = 0xBF;
            produced = 2;
        }

        if (written + produced + 1 > dst_capacity) {
            return GBC_E_BUFFER_TOO_SMALL;
        }
        dst[written] = encoded[0];
        if (produced > 1) {
            dst[written + 1] = encoded[1];
        }
        written += produced;
        offset += consumed;
    }

    if (dst_capacity > 0) {
        dst[written] = 0;
    }
    *out_length = written;
    return GBC_OK;
}
```

C++ 封装同样不替换：`gbcodec::to_gb2312` 抛出 `gbcodec::error`（`status()` 可能是
`GBC_E_UNMAPPABLE`，也可能是其它错误），`gbcodec::try_to_gb2312` 返回 `false`，
并把原因写入可选的 `gbc_status`。

## 与 UTF-16 的关系

GB2312 只覆盖基本多文种平面（BMP，`U+0000`–`U+FFFF`），因此：

- GB2312 → UTF-16 时不会产生代理项对；
- UTF-16 → GB2312 时，代理项对（`U+10000` 以上）一律无法映射；
- UTF-8 的 4 字节序列（`U+10000`–`U+10FFFF`）同样无法映射。

## 字节序

GB2312 是字节流格式，没有字节序的概念。`gbcodec` 的所有 GB2312 接口都使用
`uint8_t` / `char` 缓冲区，按「首字节、尾字节」的顺序处理；不要把它当作
`uint16_t` 数组，否则在大端与小端平台上会得到不同结果（历史实现就有这个问题）。

## 校验策略

- UTF-8 按 RFC 3629 严格校验：拒绝超长编码、代理项 `U+D800..U+DFFF`、
  超过 `U+10FFFF` 的码点、非法首字节与非法续字节。
- UTF-16 拒绝孤立的高/低代理项；截断的代理项对返回 `GBC_E_TRUNCATED`。
- GB2312 校验首尾字节范围，并区分「非法字节」（`GBC_E_INVALID_SEQUENCE`）与
  「合法但未分配」（`GBC_E_UNMAPPABLE`）。
- 输入长度不足时返回 `GBC_E_TRUNCATED`，不会越界读取。

## 不在 1.0 范围内

- GBK / CP936 / GB18030 编码输出；
- Big5、Shift-JIS 等其它东亚字符集；
- 基于 `setlocale` / `MultiByteToWideChar` 的系统 API 封装。

如果需要这些能力，建议在调用方组合使用其它库，`gbcodec` 只作为 GB2312 这一
层的稳定实现。

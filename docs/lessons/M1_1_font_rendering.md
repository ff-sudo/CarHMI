# M1.1 中文/阿拉伯文字体渲染修复 — 经验总结

## 问题现象

切换到中文或阿拉伯语后，界面文字完全不显示（空白）。

## 根因分析

问题分三层，需要逐层排查：

### 第一层：字体不含目标字形

默认加载 `arial.ttf`，该字体只含拉丁字符。`stbtt_FindGlyphIndex()` 对中文/阿拉伯 codepoint 返回 0，BakeGlyph 直接跳过。

**教训**：字体文件的字形覆盖范围是最先要确认的事，通过日志 `no glyph for U+XXXX` 可以快速定位。

### 第二层：切语言未切字体

I18n 系统的 `SetLanguage()` 只改翻译文本和方向，没有触发字体切换。

**解决**：在语言 JSON 中加 `"font"` 字段，`LanguageChangedEvent` 携带 fontPath，Application 订阅事件自动 ReloadFont。

### 第三层：Widget 文本未刷新

Label/Button 在 Scene::OnEnter() 时用 `I18n::Get().T(key)` 取了文本值，但切语言后没有重新获取。

**解决**：新增 `SetI18nKey()` 方法，内部订阅 `LanguageChangedEvent`，切语言时自动用新翻译更新文本。

## 踩过的坑

### 1. 构建目录的资源副本不会自动同步

CMake 的 `POST_BUILD copy_directory` 只在 target 重新链接时触发。修改 JSON/字体资源文件后如果没有代码改动，需要手动拷贝或 touch 一下源文件触发重编译。

**应对**：后续可以改用 symlink 或在 CMake 中加 `DEPENDS` 确保资源文件变更被检测。

### 2. .ttc 字体集合的 font index

msyh.ttc 是字体集合文件，`stbtt_GetFontOffsetForIndex(data, 0)` 返回 offset=32 而非 0。代码已正确处理（fontOffset=32 后传给 InitFont），但如果遇到别的 .ttc 可能需要尝试不同 index。

### 3. 字形覆盖范围因字体而异

- `arial.ttf`：仅拉丁
- `msyh.ttc`：拉丁 + 中日韩，不含阿拉伯
- `tahoma.ttf`：拉丁 + 阿拉伯 + 希伯来等

没有一个字体能覆盖所有语言。长期方案应该是 Fallback 字体链或使用 Noto Sans 系列。

### 4. 排查渲染问题的分层方法

正确的排查顺序：
1. 日志确认字形是否找到（stbtt_FindGlyphIndex 返回值）
2. 确认 atlas bitmap 是否写入了像素（RebuildAtlas 是否被调用）
3. 硬编码一个 DrawText 直接渲染目标文字，隔离是字体层还是 Widget 层的问题
4. 最后再查 Widget 的文本更新逻辑

## 架构决策记录

| 决策 | 选择 | 理由 |
|------|------|------|
| 字体切换方式 | 语言-字体映射（JSON 配置） | 比 fallback 链简单，比硬编码灵活 |
| i18n 响应方式 | Widget 主动订阅事件 | 不需要改 Widget 基类，按需接入 |
| 阿拉伯字体 | tahoma.ttf | Windows 自带，体积小，覆盖阿拉伯字形 |

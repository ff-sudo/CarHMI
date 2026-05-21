# M1.5 TextInput 控件 — 经验总结

## 实现方案

### 基础设施补齐

TextInput 需要 3 个基础设施补齐：

1. **TextInputEvent + SDL_TEXTINPUT**：SDL_TEXTINPUT 提供 IME 感知的 UTF-8 文本，是国际化文本输入的正确方式。KeyEvent 的 scancode 只能处理英文按键，不能处理中文/日文等 IME 输入。SDL_StartTextInput() 在窗口创建后全局启用。

2. **Font.MeasureSubstring(text, charCount)**：量测前 N 个 UTF-8 字符的像素宽度。复用内部的 DecodeUTF8 + GetOrLoadGlyph 循环。用于光标 X 坐标定位和鼠标点击→光标位置转换。

3. **Widget.OnKeyEvent()**：返回 bool 的虚方法。FocusManager 在处理 Tab/Arrow 之前先调用 focusedWidget->OnKeyEvent()，返回 true 表示事件已被控件消费，FocusManager 不再处理。这样 TextInput 可以拦截左右箭头用于光标移动，而不被 FocusManager 当作焦点切换。

### TextInput 核心逻辑

- **文本存储**：std::string（UTF-8），光标位置是字符索引（不是字节索引）
- **光标闪烁**：Update 中累加时间，每 0.5s 翻转可见性
- **文本输入**：订阅 TextInputEvent，在光标位置插入 UTF-8 文本
- **编辑键**：通过 OnKeyEvent 处理 Backspace/Delete/Arrow/Home/End
- **鼠标定位**：点击时遍历每个字符的累积宽度，找最近的位置
- **裁剪**：PushClipRect 限制文本绘制区域

## 踩过的坑

### 1. UTF-8 字符索引 vs 字节索引

std::string 是字节序列。光标位置（m_cursorPos）是**字符索引**，但 string::insert/erase 操作需要**字节索引**。必须用 CharIndexToByteIndex() 转换。

中文字符占 3 字节，emoji 占 4 字节。如果直接用字节索引做光标位置，Backspace 只删一个字节会破坏 UTF-8 序列。

### 2. SDL_TEXTINPUT vs SDL_KEYDOWN 的分工

- SDL_KEYDOWN (KeyEvent) 用于编辑操作：Backspace、Delete、Arrow、Home、End、Enter
- SDL_TEXTINPUT (TextInputEvent) 用于文本插入：字母、数字、中文、符号

不要在 KeyEvent 中处理字母输入！SDL_TEXTINPUT 已经处理了 shift/caps 等修饰键，直接给出正确的字符。

### 3. FocusManager 事件消费机制

TextInput 需要拦截左右箭头键做光标移动，但 FocusManager 默认用左右箭头切焦点。解决方案：OnKeyEvent 返回 true 时 FocusManager 跳过自己的处理。

这个设计让 FocusManager 和 Widget 形成了"先到先得"的事件消费链。

### 4. Theme 中缺少 accentColor 的统一访问

TextInput 需要聚焦边框颜色。WidgetStyle 有 accentColor 但 Theme 不包含 WidgetStyle 实例。最终硬编码 {0.2, 0.6, 0.9, 1.0}。后续应统一 Theme 的 accent color 访问。

## 架构决策记录

| 决策 | 选择 | 理由 |
|------|------|------|
| 文本事件用 SDL_TEXTINPUT | 是 | IME 感知、国际化正确、修饰键自动处理 |
| 光标位置用字符索引 | 是 | 直觉正确，UTF-8 字节操作通过转换函数隔离 |
| OnKeyEvent 返回 bool | 是 | 事件消费机制，避免 TextInput 和 FocusManager 冲突 |
| SDL_StartTextInput 全局启用 | 是 | 简化处理，不需要按 focus 切换（后续可优化为按需启停） |
| 光标闪烁用 Update 累加 | 是 | 不依赖 TimerManager，避免 Scene 切换时定时器残留 |

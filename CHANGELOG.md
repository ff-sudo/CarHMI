# Changelog

本文件记录 CarHMI 平台每个版本的主要变更。

格式基于 [Keep a Changelog](https://keepachangelog.com/zh-CN/1.0.0/)，
版本号遵循 [语义化版本](https://semver.org/lang/zh-CN/)。

---

## [0.2.8] - 2026-05-21

### Added
- **TextInput 控件**（`gui/widgets/text_input.h`）：单行文本输入框
  - 光标闪烁、左右箭头/Home/End 移动光标
  - Backspace/Delete 删除、Enter 提交
  - 鼠标点击定位光标、Scissor 裁剪溢出文本
  - Placeholder 提示文本、聚焦边框高亮
- **TextInputEvent**：SDL_TEXTINPUT 事件映射，支持 UTF-8 文本输入
- **Font.MeasureSubstring()**：量测前 N 个字符宽度（用于光标定位）
- **Widget.OnKeyEvent()**：虚方法，FocusManager 路由键盘事件到聚焦控件
- **FocusManager.GetFocusChain()**：公开焦点链访问
- **TextInputDemoScene**：3 个输入框交互式演示

### Changed
- FocusManager 键盘处理：先转发 OnKeyEvent 到聚焦控件，返回 true 则消费事件
- SDL2 平台：启用 SDL_StartTextInput，处理 SDL_TEXTINPUT 事件

---

## [0.2.7] - 2026-05-21

### Added
- **ScrollView 控件**（`gui/widgets/scroll_view.h`）：通用滚动容器
  - 垂直 + 水平双向滚动
  - Scissor 裁剪溢出内容，内容区与滚动条不重叠
  - 鼠标滚轮悬停即滚（无需 focus）
  - 滚动条可点击拖拽，拖拽时高亮反馈
  - 自动计算内容尺寸，自动显示/隐藏对应方向滚动条
- **ScrollDemoScene**：ScrollView 交互式演示（20 个子控件 + ImGui 调参）

---

## [0.2.6] - 2026-05-21

### Added
- **Canvas 自绘 API**（`gui/canvas.h`）：高层绘图上下文，包装 BatchRenderer2D
  - 状态栈：Save/Restore + Translate
  - 样式：SetFillColor/SetStrokeColor/SetStrokeWidth
  - 矩形：FillRect/StrokeRect/FillRoundRect
  - 圆形：FillCircle/StrokeCircle/FillArc/StrokeArc
  - 线条：DrawLine/DrawPolyline
  - 路径：BeginPath/MoveTo/LineTo/ClosePath/StrokePath/FillPath
  - 文本：DrawText/MeasureText
  - 裁剪：ClipRect/ResetClip
- **CanvasWidget**（`gui/widgets/canvas_widget.h`）：基于 callback 的自绘控件
- **BatchRenderer2D.DrawTriangle**：三角形绘制原语（支持 Canvas FillPath）
- **CanvasDemoScene**：Canvas API 全功能交互式演示（ImGui 实时调参）

---

## [0.2.5] - 2026-05-21

### Added
- **TimerManager**（`core/timer/timer_manager.h`）：全局定时器管理器单例
  - `SetTimeout(callback, delayMs)` — 一次性延时执行
  - `SetInterval(callback, intervalMs)` — 周期重复执行
  - 返回 Connection（RAII 自动取消）
  - API 使用 int 毫秒，内部 float 累加避免精度丢失
- **TimerDemoScene**：交互式定时器演示（ImGui 面板创建/取消/监控定时器）

---

## [0.2.4] - 2026-05-21

### Added
- **Insets 结构体**：四边独立值（top/right/bottom/left），用于 margin 和 padding
- **SizePolicy**：支持 Fixed/Percent/Fill 三种尺寸模式
- **Widget.margin**：所有控件支持 SetMargin(Insets)
- **Widget.SizePolicy**：支持 SetWidthPercent/SetFillWidth 等便捷方法
- **BoxLayout.Padding 升级**：从 float 升级为 Insets，支持四边独立 padding
- **BoxLayout 三轮 Recalculate**：测量 Fixed → 分配 Percent/Fill → 放置位置
- **DebugPanel 模块**（`gui/debug/debug_panel.h`）：通用 ImGui 调试组件集
  - ShowRenderStats / ShowThemeSelector / ShowLanguageSelector / ShowSceneInfo
  - EditInsets / EditWidget / EditBoxLayout / ShowWidgetTree
- **LayoutDemoScene**：交互式布局调参 Demo（ImGui 实时编辑 padding/margin/sizePolicy）
- **SceneManager 延迟 Pop**：防止 OnImGui 中调 Pop 导致 use-after-free 崩溃

### Fixed
- 修复水平布局 Percent 子控件溢出（Percent 现在基于去掉 Fixed 后的剩余空间计算）
- 修复 OnImGui 中点击 Back 按钮导致崩溃（SceneManager::Pop 改为延迟执行）

### Changed
- Dashboard 按钮从单行改为 2×3 网格布局，按钮宽度自动填充
  - EditInsets / EditWidget / EditBoxLayout / ShowWidgetTree
- **LayoutDemoScene**：交互式布局调参 Demo（ImGui 实时编辑 padding/margin/sizePolicy）
- BoxLayout::Recalculate() 支持子控件 margin 和 SizePolicy

---

## [0.2.3] - 2026-05-21

### Added
- **DrawRoundedRect**：BatchRenderer2D 新增圆角矩形绘制（CPU 几何细分，9-patch + 4角扇形）
- **Scissor 裁剪栈**：BatchRenderer2D 新增 PushScissor/PopScissor，支持嵌套裁剪
- **SetViewportSize**：BatchRenderer2D 设置视口尺寸（用于 Scissor Y 轴翻转）
- **UIContext.PushClipRect/PopClipRect**：GUI 层裁剪便捷接口
- **RenderDemoScene**：Gallery 新增渲染 Demo 场景，展示圆角矩形 + Scissor 裁剪

### Changed
- CMakeUserPresets.json 添加 CMAKE_RC_COMPILER 完整路径，修复 CLI 构建找不到 rc.exe

---

## [0.2.2] - 2026-05-19

### Added
- **语言-字体映射**：I18n 语言 JSON 支持 `"font"` 字段，切语言时自动切换字体
- **Label.SetI18nKey() / Button.SetI18nKey()**：绑定翻译 key，切语言自动刷新文本
- **LanguageChangedEvent.fontPath**：事件携带字体路径信息

### Fixed
- 修复中文/阿拉伯文字体不显示（根因：默认 arial.ttf 不含非拉丁字形）
- 修复切语言后 Widget 文本不更新（Label/Button 未订阅 LanguageChangedEvent）

### Changed
- 阿拉伯语使用 tahoma.ttf（含阿拉伯字形），中文使用 msyh.ttc

---

## [0.2.1] - 2026-05-18

### Added
- **Connection 类**（RAII）：析构自动断开事件订阅/属性回调
- **ConnectionGroup**：批量管理多个 Connection
- **EventBus.PostDeferred()**：线程安全延迟投递，下一帧主线程分发
- **EventBus.FlushDeferred()**：主循环每帧 flush 延迟队列
- **Property.SetDeferred()**：脏标记模式，帧末批量通知
- **Property.SetFromThread()**：线程安全写入（mutex 保护）
- **Property 循环检测**：A→B→A 通知链自动截断
- **BidirectionalBinding**：双向绑定 + 循环防护
- **BindingScope**：作用域绑定，析构自动解除
- **ResourceManager**：统一资源加载 + 缓存 + 引用计数

### Changed
- EventBus.Subscribe() 返回 Connection（旧 subscribe 保留为兼容别名）
- Property.OnChanged() 返回 Connection（不再返回 int）
- Widget/FocusManager/UIContext 改用 ConnectionGroup 管理事件订阅

### Fixed
- ImGui 调试面板无法点击（Connection RAII 导致订阅立即断开）
- 3D 场景鼠标拖拽失效（同上原因）
版本号遵循 [语义化版本](https://semver.org/lang/zh-CN/)。

---

## [0.2.0] - 2026-05-18

### Changed
- **架构重构**：从单一 src/ 拆分为 7 个独立 CMake 模块库
  - PAL（平台抽象）、RHI（渲染抽象）、Core（事件/属性/动画/场景）
  - GUI（控件/布局/样式/i18n）、Renderer3D
  - 预留：Runtime、Script、Pipeline
- 引入分层子命名空间（CarHMI::PAL / RHI / Core / GUI / Render3D）
- 演示程序重命名为 Gallery（apps/gallery/）
- 新增 CMakePresets.json（dev-win / dev-linux / release）
- 新增模块宏 cmake/CarHMIModule.cmake

### Fixed
- .gitignore 增加 3D 资产 .obj 文件例外

---

## [0.1.0] - 2026-05-17

### Added
- 初始项目：SDL2 + OpenGL 3.3 + ImGui
- 2D BatchRenderer（矩形/圆/弧/线/纹理）
- 3D Renderer（Blinn-Phong + OBJ 加载）
- 11 个 GUI 控件（Label, Button, Slider, ProgressBar, Panel, Gauge, Knob, Toggle, TabBar, ListWidget）
- BoxLayout（VBox/HBox，RTL 适配）
- FocusManager（Tab/方向键/滚轮导航）
- 属性系统（Property<T> / PropertyMap / Binding）
- 动画系统（Tween / PropertyTween / Sequence / Parallel / Easing）
- 样式系统（JSON 主题 + 热重载 + ThemeManager）
- 国际化（I18n 资源管理 + SheenBidi BiDi + RTL 布局）
- 场景管理（页面栈 push/pop/replace + 过渡动画）
- EventBus（基于 eventpp，SubscriptionHandle 自动取消）

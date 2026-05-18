# Changelog

本文件记录 CarHMI 平台每个版本的主要变更。

格式基于 [Keep a Changelog](https://keepachangelog.com/zh-CN/1.0.0/)，
版本号遵循 [语义化版本](https://semver.org/lang/zh-CN/)。

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

# CarHMI 项目指南

## 项目概述

车机 HMI 开发平台，C++17，分层模块化架构。目标是覆盖从平台抽象到 IDE 工具的全链路。

## 构建

```bash
# Windows (MSVC + Ninja，使用 CMakeUserPresets.json 中的 dev-win-cvte preset)
cmake --preset dev-win-cvte
cmake --build build/dev-win

# CLI 构建需要设置 MSVC 环境变量（INCLUDE/LIB/PATH），详见 CMakeUserPresets.json
# 运行
./build/dev-win/apps/gallery/Gallery.exe
```

## 项目结构

```
modules/pal/         # L1 平台抽象 (CarHMI::PAL) — SDL2 后端
modules/rhi/         # L2 渲染抽象 (CarHMI::RHI) — OpenGL 3.3, BatchRenderer2D
modules/core/        # L3 核心运行时 (CarHMI::Core) — EventBus/Property/Animation/Scene/ResourceManager/TimerManager
modules/gui/         # L4 GUI 框架 (CarHMI::GUI) — Widget/控件/布局/样式/i18n/Canvas/DebugPanel
modules/renderer3d/  # L2+ 3D 渲染 (CarHMI::Render3D)
modules/runtime/     # L5 预留
modules/script/      # L6 预留
modules/pipeline/    # L7 预留
apps/gallery/        # 演示程序 (CarHMI::Gallery)
docs/lessons/        # 各里程碑踩坑经验
```

## 关键规则

- **依赖方向**：只能上层依赖下层（GUI→Core→PAL），禁止反向 include
- **模块公共 API**：通过 `modules/<name>/include/<name>/` 暴露，`src/` 内部不可外部访问
- **新模块**：使用 `cmake/CarHMIModule.cmake` 中的 `carhmi_add_module()` 宏
- **命名空间**：每层有独立子命名空间 CarHMI::PAL/RHI/Core/GUI/Render3D
- **事件订阅必须保存 Connection**：`EventBus::Subscribe()` 和 `Property::OnChanged()` 返回 RAII Connection，丢弃即断开
- **线程安全写入**：从非主线程写属性用 `SetFromThread()`，发事件用 `PostDeferred()`
- **Widget 键盘事件**：`OnKeyEvent()` 返回 true 表示消费事件，FocusManager 不再处理

## 事件系统用法

```cpp
// 订阅（Connection 必须保存！）
ConnectionGroup m_connections;
m_connections.Add(EventBus::Get().Subscribe<Event>([](auto& e) { ... }));

// 发布
EventBus::Get().Post(MyEvent{...});         // 主线程同步
EventBus::Get().PostDeferred(MyEvent{...}); // 跨线程安全
```

## 属性系统用法

```cpp
Property<float> speed("speed", 0.0f);
Connection c = speed.OnChanged([](auto& old, auto& now) { ... });
speed.Set(120.0f);           // 立即通知
speed.SetDeferred(120.0f);   // 帧末批量通知
speed.SetFromThread(120.0f); // 线程安全
```

## 定时器用法

```cpp
// SetTimeout（一次性，毫秒）
Connection c = TimerManager::Get().SetTimeout([]() { ... }, 2000);
// SetInterval（周期，毫秒）
Connection c = TimerManager::Get().SetInterval([]() { ... }, 500);
// 丢弃 Connection 或 Disconnect 即取消
```

## 布局系统

```cpp
// BoxLayout 支持 Insets padding + 子控件 margin + SizePolicy
auto* layout = new BoxLayout(id, pos, size, BoxDirection::Vertical, 20.0f, 10.0f);
child->SetFillWidth();                    // 填满父容器宽度
child->SetWidthPercent(80.0f);            // 占可用宽度的 80%
child->SetMargin(Insets(5, 10, 5, 10));   // 四边独立 margin
```

## Canvas 自绘 API

```cpp
Canvas cv(ctx.GetRenderer(), ctx.GetFont(), GetAbsolutePos());
cv.SetFillColor({1, 0, 0, 1});
cv.FillRoundRect(0, 0, 100, 50, 10);
cv.BeginPath(); cv.MoveTo(0, 0); cv.LineTo(50, 50); cv.StrokePath();
```

## 开发约定

- 提交信息用中文，简洁描述"做了什么"
- 分支：develop_work 日常开发，main 保持稳定
- 里程碑完成打 tag（v0.3.0 等）
- 参考 `docs/ROADMAP.md` 确定当前要做什么
- 参考 `CHANGELOG.md` 了解历史变更
- 踩坑经验记录到 `docs/lessons/`

## 已知问题

- LanguageChangedEvent 未被所有 Widget 订阅（仅 DashboardScene 的 Label/Button 接入了 i18n key，其他页面需手动接入）
- Canvas FillPath 仅支持凸多边形（triangle fan）
- TextInput 未实现文本选择/复制粘贴（基础版）
- Theme 缺少统一的 accentColor 访问（TextInput 聚焦色硬编码）

## 第三方依赖

SDL2, GLAD (OpenGL 3.3), GLM, spdlog, eventpp, ImGui, nlohmann/json, stb_image/stb_truetype, tinyobjloader, SheenBidi

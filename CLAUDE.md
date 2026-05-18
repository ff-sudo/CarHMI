# CarHMI 项目指南

## 项目概述

车机 HMI 开发平台，C++17，分层模块化架构。目标是覆盖从平台抽象到 IDE 工具的全链路。

## 构建

```bash
cmake --preset dev-win      # Windows Debug (Ninja)
cmake --build build/dev-win
# 运行
./build/dev-win/apps/gallery/Gallery.exe
```

## 项目结构

```
modules/pal/         # L1 平台抽象 (CarHMI::PAL) — SDL2 后端
modules/rhi/         # L2 渲染抽象 (CarHMI::RHI) — OpenGL 3.3
modules/core/        # L3 核心运行时 (CarHMI::Core) — EventBus/Property/Animation/Scene/ResourceManager
modules/gui/         # L4 GUI 框架 (CarHMI::GUI) — Widget/控件/布局/样式/i18n
modules/renderer3d/  # L2+ 3D 渲染 (CarHMI::Render3D)
modules/runtime/     # L5 预留
modules/script/      # L6 预留
modules/pipeline/    # L7 预留
apps/gallery/        # 演示程序 (CarHMI::Gallery)
```

## 关键规则

- **依赖方向**：只能上层依赖下层（GUI→Core→PAL），禁止反向 include
- **模块公共 API**：通过 `modules/<name>/include/<name>/` 暴露，`src/` 内部不可外部访问
- **新模块**：使用 `cmake/CarHMIModule.cmake` 中的 `carhmi_add_module()` 宏
- **命名空间**：每层有独立子命名空间 CarHMI::PAL/RHI/Core/GUI/Render3D
- **事件订阅必须保存 Connection**：`EventBus::Subscribe()` 和 `Property::OnChanged()` 返回 RAII Connection，丢弃即断开
- **线程安全写入**：从非主线程写属性用 `SetFromThread()`，发事件用 `PostDeferred()`

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

## 开发约定

- 提交信息用中文，简洁描述"做了什么"
- 分支：develop 日常开发，main 保持稳定
- 里程碑完成打 tag（v0.3.0 等）
- 参考 `docs/ROADMAP.md` 确定当前要做什么
- 参考 `CHANGELOG.md` 了解历史变更

## 已知问题

- 中文/阿拉伯文字体渲染不显示（stbtt 字形加载问题）
- LanguageChangedEvent 未被 Widget 订阅（语言切换需重进 Scene）

## 第三方依赖

SDL2, GLAD (OpenGL 3.3), GLM, spdlog, eventpp, ImGui, nlohmann/json, stb_image/stb_truetype, tinyobjloader, SheenBidi

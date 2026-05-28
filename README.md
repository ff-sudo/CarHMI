# CarHMI — 车机 HMI 开发平台

[![Build](https://github.com/hyp-cvte/CarHMI/actions/workflows/build.yml/badge.svg)](https://github.com/hyp-cvte/CarHMI/actions/workflows/build.yml)
[![Tests](https://img.shields.io/badge/tests-67%20passed-brightgreen)](tests/)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue)](https://en.cppreference.com/w/cpp/17)
[![License](https://img.shields.io/badge/license-MIT-green)](LICENSE)

C++17 分层模块化车机 HMI 框架，覆盖从平台抽象到 GUI 控件的全链路。

## 快速开始

```bash
# 环境：Windows + Visual Studio 2022 + Ninja
cmake --preset dev-win-cvte
cmake --build build/dev-win

# 运行
./build/dev-win/apps/gallery/Gallery.exe

# 运行测试
cmake --build build/dev-win --target carhmi_core_tests carhmi_gui_tests
cd build/dev-win && ctest --output-on-failure
```

## 项目结构

```
modules/pal/         # L1 平台抽象 — SDL2 后端
modules/rhi/         # L2 渲染抽象 — OpenGL 3.3, BatchRenderer2D
modules/core/        # L3 核心运行时 — EventBus/Property/Animation/Scene/Resource/Timer
modules/gui/         # L4 GUI 框架 — 14 个控件/布局/样式/i18n/Canvas/DebugPanel
modules/renderer3d/  # L2+ 3D 渲染
modules/runtime/     # L5 预计 M2： JSON 场景加载
modules/script/      # L6 预计 M3： Lua 脚本引擎
modules/pipeline/    # L7 预计 M5： 资源管线
apps/gallery/        # 演示程序 — 9 个交互式 Demo 场景
```

## 技术栈

SDL2 · OpenGL 3.3 (GLAD) · GLM · spdlog · eventpp · ImGui · nlohmann/json · SheenBidi · stb · tinyobjloader · GoogleTest

## 文档

- [CLAUDE.md](CLAUDE.md) — 项目指南与 API 参考
- [CHANGELOG.md](CHANGELOG.md) — 版本变更记录
- [docs/ROADMAP.md](docs/ROADMAP.md) — 开发路线图
- [docs/](docs/) — 设计文档（15+ 篇）
- [docs/lessons/](docs/lessons/) — 踩坑经验（7 篇，含 28 条架构决策记录）
- [CONTRIBUTING.md](CONTRIBUTING.md) — 贡献指南

## 开发状态

当前版本 **v0.2.8**，Milestone 1（基础加固）完成。

## 许可证

MIT License — 详见 [LICENSE](LICENSE)

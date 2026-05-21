# CarHMI 开发路线图

> 里程碑按功能划分，不绑定固定时间。每个里程碑完成后打 tag、更新 CHANGELOG。

---

## 当前版本：v0.2.0（架构重构）✅

---

## Milestone 1：基础加固

**目标**：修复已知问题，补齐基础能力，让框架达到"可用于构建真实 Demo"的状态。

| # | 任务 | 模块 | 优先级 |
|---|------|------|--------|
| 1.1 | ~~修复中文/阿拉伯文字体不显示~~ | RHI | ✅ |
| 1.2 | ~~ResourceManager 基础版（统一加载 + 引用计数）~~ | Core | ✅ |
| 1.3 | ~~渲染增强：圆角矩形、裁剪区域（Scissor）~~ | RHI | ✅ |
| 1.4 | ScrollView 控件 | GUI | P1 |
| 1.5 | TextInput 控件 | GUI | P1 |
| 1.6 | ImageView 控件（支持背景图） | GUI | P2 |
| 1.7 | Dialog/Toast 弹窗系统 | GUI | P2 |
| 1.8 | ~~定时器/调度器（setTimeout/setInterval 语义）~~ | Core | ✅ |
| 1.9 | ~~布局增强：Margin/Padding 统一模型 + 百分比尺寸~~ | GUI | ✅ |
| 1.10 | Canvas 自绘 API 基础版（DrawRoundRect/DrawPath） | RHI/GUI | P1 |

**交付物**：Gallery 中有一个"控件全景 Demo"展示所有控件，中文正常显示。

---

## Milestone 2：运行时引擎

**目标**：可以从 JSON 文件加载整个 UI 场景，不需要硬编码 C++。

| # | 任务 | 模块 | 优先级 |
|---|------|------|--------|
| 2.1 | 场景描述格式定义（scene.json → Widget 树） | Runtime | P0 |
| 2.2 | Widget 序列化/反序列化 | Runtime | P0 |
| 2.3 | DataModel 抽象（key-value 数据源） | Runtime | P1 |
| 2.4 | 模拟数据源（用于开发测试） | Runtime | P1 |
| 2.5 | DataModel → Widget 属性自动绑定 | Runtime | P1 |
| 2.6 | 状态机（页面导航 + 条件跳转） | Runtime | P1 |

**交付物**：Gallery 中有一个"JSON 加载 Demo"，修改 JSON 即可改变 UI。

---

## Milestone 3：脚本引擎

**目标**：业务逻辑可以用 Lua 编写，支持热重载。

| # | 任务 | 模块 | 优先级 |
|---|------|------|--------|
| 3.1 | Lua + sol2 集成 | Script | P0 |
| 3.2 | Widget API 绑定（get/set 属性、事件监听） | Script | P0 |
| 3.3 | 场景脚本生命周期（OnEnter/OnExit/OnUpdate） | Script | P1 |
| 3.4 | 脚本热重载（文件监听 + 重新执行） | Script | P1 |
| 3.5 | 动画 API 绑定 | Script | P2 |

**交付物**：Gallery 中有一个"脚本 Demo"，修改 Lua 文件后 UI 实时变化。

---

## Milestone 4：RHI 抽象

**目标**：渲染代码与 OpenGL 解耦，为多后端铺路。

| # | 任务 | 模块 | 优先级 |
|---|------|------|--------|
| 4.1 | RHI 接口定义（Device/CommandBuffer/Pipeline） | RHI | P0 |
| 4.2 | OpenGL 3.3 后端适配到 RHI 接口 | RHI | P0 |
| 4.3 | Shader 跨平台编译（GLSL → SPIR-V） | RHI | P2 |
| 4.4 | OpenGL ES 3.0 后端（预留） | RHI | P2 |

**交付物**：现有功能在新 RHI 接口下正常运行，渲染代码不再直接调用 GL。

---

## Milestone 5：资源管线 + 工具

**目标**：资源有打包/索引/热重载能力，初步的可视化工具。

| # | 任务 | 模块 | 优先级 |
|---|------|------|--------|
| 5.1 | 资源 URI 寻址（res://textures/btn.png） | Pipeline | P0 |
| 5.2 | 图集打包器（多张小图 → Atlas） | Pipeline | P1 |
| 5.3 | 资源打包格式（.pak 二进制包） | Pipeline | P2 |
| 5.4 | HMI Studio 原型（ImGui 编辑器） | Tools | P2 |

---

## 远期（Milestone 6+）

- Win32 原生后端（不依赖 SDL2）
- Wayland 后端
- Vulkan 后端
- 触屏/手势系统
- CAN/SOME-IP 信号接入
- 嵌入式目标（QNX/Android）部署

---

## 开发约定

### 版本号规则
- **x.Y.0** — 完成一个 Milestone 时升 minor 版本
- **x.y.Z** — Milestone 内的 bug 修复或小改进升 patch

### 工作流
1. 开始一个任务前，在本文件对应任务后标注 `🔨`
2. 完成后标注 `✅`，同时更新 CHANGELOG.md
3. 一个 Milestone 全部完成后，打 git tag（如 `v0.3.0`）
4. 遇到计划外的 bug/需求，追加到对应 Milestone 表格末尾

### 优先级定义
- **P0**：阻塞后续任务，必须先做
- **P1**：Milestone 核心功能
- **P2**：Nice to have，可推迟到下个 Milestone

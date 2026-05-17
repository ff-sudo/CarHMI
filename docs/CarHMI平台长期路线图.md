# CarHMI 平台长期路线图

> 目标：构建完整的车机 HMI 开发平台，覆盖从平台抽象到 IDE 工具的全链路。

---

## 架构总览

```
┌──────────────────────────────────────────────────────────────┐
│  IDE 工具层        HMI Studio（UI 编辑器 + 仿真 + 调试）      │
├──────────────────────────────────────────────────────────────┤
│  资源管线          ResourcePipeline（打包/索引/热重载/版本）    │
├──────────────────────────────────────────────────────────────┤
│  脚本业务层        ScriptEngine（Lua/JS 业务逻辑 + API 绑定） │
├──────────────────────────────────────────────────────────────┤
│  HMI Runtime      项目加载/场景调度/生命周期/信号连接          │
├──────────────────────────────────────────────────────────────┤
│  HMI Framework    控件/布局/样式/动画/数据绑定/焦点/i18n       │
├──────────────────────────────────────────────────────────────┤
│  Core Runtime     场景图/事件总线/属性系统/资源管理/序列化      │
├──────────────────────────────────────────────────────────────┤
│  RHI 渲染抽象     OpenGL 3.3 / OpenGL ES 3.0 / Vulkan       │
├──────────────────────────────────────────────────────────────┤
│  PAL 平台抽象     Win32 / X11 / Wayland / QNX / Android      │
└──────────────────────────────────────────────────────────────┘
```

---

## Layer 1: PAL — 平台抽象层

将操作系统差异隔离在一层接口之后。

### 已完成
- [x] Platform 接口定义（`platform.h`）
- [x] SDL2 后端实现
- [x] 窗口创建/销毁/resize/fullscreen
- [x] 输入事件 → EventBus 转发

### 待实现
- [ ] **Win32 原生后端** — 不依赖 SDL2，直接用 Win32 API
  - [ ] 窗口创建（CreateWindowEx + WGL）
  - [ ] 消息循环（WM_KEYDOWN/WM_MOUSEMOVE 等）
  - [ ] 高 DPI 支持（DPI awareness）
- [ ] **X11 后端** — Linux 桌面
  - [ ] Xlib/XCB 窗口
  - [ ] GLX 上下文
  - [ ] XInput2 输入
- [ ] **Wayland 后端** — 现代 Linux 显示协议
  - [ ] wl_surface + EGL
  - [ ] xdg_shell 窗口管理
  - [ ] libinput 输入
- [ ] **QNX 后端** — 车机主流 RTOS
  - [ ] Screen 窗口系统
  - [ ] EGL 上下文
- [ ] **多窗口支持** — 一个 Application 管理多个窗口
- [ ] **多显示器支持** — 仪表盘 + 中控双屏
- [ ] **触屏输入** — 多点触控、手势识别
- [ ] **旋钮/按键硬件输入** — HID 设备抽象

---

## Layer 2: RHI — 渲染硬件抽象

将图形 API 差异隔离，上层代码不直接调用 GL/Vulkan。

### 已完成
- [x] OpenGL 3.3 Core 直接调用（Shader/Texture/VAO/VBO）
- [x] 2D BatchRenderer（矩形/圆/圆弧/线段/纹理）
- [x] 3D Renderer（Blinn-Phong + OBJ 加载）

### 待实现
- [ ] **RHI 接口定义** — 抽象渲染命令
  - [ ] `RHIDevice` — 创建缓冲/纹理/Shader/管线
  - [ ] `RHICommandBuffer` — 记录渲染命令
  - [ ] `RHITexture / RHIBuffer / RHIShader` — 资源抽象
  - [ ] `RHIPipeline` — 渲染管线状态封装
- [ ] **OpenGL 3.3 后端** — 当前代码迁移到 RHI 接口
- [ ] **OpenGL ES 3.0 后端** — 嵌入式 GPU（高通 Adreno、ARM Mali）
  - [ ] GLSL 300 es 适配
  - [ ] EGL 上下文
  - [ ] 纹理压缩格式（ETC2/ASTC）
- [ ] **Vulkan 后端** — 高性能渲染
  - [ ] Instance/Device/Queue 初始化
  - [ ] Swapchain 管理
  - [ ] Command Buffer 录制
  - [ ] Descriptor Set 管理
  - [ ] SPIR-V Shader 编译
- [ ] **Shader 跨平台** — GLSL → SPIR-V 编译管线（glslang/shaderc）

---

## Layer 3: Core Runtime — 核心运行时

框架的基础设施，所有上层模块依赖它。

### 已完成
- [x] EventBus（eventpp，SubscriptionHandle 自动取消）
- [x] Property<T> + PropertyMap + Binding
- [x] AnimationManager（Tween/PropertyTween/Sequence/Parallel/Easing）
- [x] SceneManager（页面栈 push/pop/replace + 过渡动画）
- [x] Application 生命周期
- [x] spdlog 日志

### 待实现
- [ ] **资源管理器（ResourceManager）**
  - [ ] 统一资源 ID（URI 或路径）
  - [ ] 引用计数 / 共享资源池
  - [ ] 异步加载（后台线程加载，主线程上传 GPU）
  - [ ] 资源类型注册表
    - [ ] 纹理（PNG/JPG/KTX/ASTC）
    - [ ] 字体（TTF/TTC/OTF）
    - [ ] Shader（GLSL/SPIR-V）
    - [ ] 3D 模型（OBJ/glTF）
    - [ ] 音频（WAV/OGG，预留）
    - [ ] 样式主题（JSON）
    - [ ] 国际化（JSON）
    - [ ] UI 布局描述（JSON/XML/自定义格式）
  - [ ] 资源热重载（文件监听 + 自动刷新）
  - [ ] 资源打包（开发时散文件，发布时打包成二进制包）
- [ ] **序列化系统**
  - [ ] Widget 树序列化/反序列化（JSON/二进制）
  - [ ] 场景描述文件格式
  - [ ] 属性快照/恢复
- [ ] **定时器/调度器** — setTimeout/setInterval 语义
- [ ] **信号/数据源** — CAN/SOME-IP/DDS 信号接入抽象

---

## Layer 4: HMI Framework — GUI 框架

车机 HMI 的 UI 组件和交互系统。

### 已完成
- [x] Widget 基类（属性系统、焦点、主题响应）
- [x] 基础控件：Label, Button, Slider, ProgressBar, Panel
- [x] 车机控件：Gauge, Knob, Toggle, TabBar, ListWidget
- [x] BoxLayout（VBox/HBox，RTL 适配）
- [x] FocusManager（Tab/方向键/滚轮导航）
- [x] UIContext（输入状态管理）
- [x] 样式系统（JSON 主题 + 热重载 + ThemeManager）
- [x] 动画系统（属性动画 + 状态过渡 + 页面过渡）
- [x] I18n + RTL（语言资源 + SheenBidi BiDi）

### 待实现
- [ ] **布局系统扩展**
  - [ ] GridLayout（网格布局）
  - [ ] StackLayout（层叠布局）
  - [ ] AnchorLayout / ConstraintLayout（约束布局）
  - [ ] 百分比/权重尺寸
  - [ ] Margin/Padding 统一模型
- [ ] **更多控件**
  - [ ] ScrollView（滚动容器）
  - [ ] TextInput（文本输入框）
  - [ ] ImageView（图片显示，支持 9-patch）
  - [ ] VideoView（视频播放，预留）
  - [ ] Map（地图控件，预留）
  - [ ] Chart（图表控件：折线/柱状/饼图）
  - [ ] Dialog / Popup / Toast（弹窗系统）
  - [ ] NavigationBar（顶部/底部导航栏）
  - [ ] Carousel（轮播图）
  - [ ] 自定义控件基类（用户可继承扩展）
- [ ] **渲染增强**
  - [ ] 圆角矩形
  - [ ] 阴影/投影
  - [ ] 模糊效果（高斯模糊背景）
  - [ ] 裁剪区域（Scissor/Stencil clipping）
  - [ ] 控件级纹理贴图（每个控件可设背景图）
- [ ] **自绘系统（Canvas / Painter API）**
  - [ ] `Canvas` 类 — 封装 2D 绘图命令的高级接口
    - [ ] `DrawRect / DrawRoundRect` — 矩形/圆角矩形
    - [ ] `DrawCircle / DrawArc / DrawRing` — 圆/弧/环
    - [ ] `DrawLine / DrawPolyline / DrawPolygon` — 线段/折线/多边形
    - [ ] `DrawPath` — 贝塞尔曲线路径（moveTo/lineTo/curveTo/close）
    - [ ] `DrawText` — 文字绘制（对齐/换行/裁剪）
    - [ ] `DrawImage / DrawImageRect` — 图片/子区域绘制
    - [ ] `DrawShadow` — 阴影绘制
  - [ ] **画笔/画刷状态**
    - [ ] `SetFillColor / SetStrokeColor` — 填充色/描边色
    - [ ] `SetStrokeWidth` — 描边宽度
    - [ ] `SetFont / SetFontSize` — 字体设置
    - [ ] `SetOpacity` — 全局透明度
    - [ ] `SetGradient` — 线性/径向渐变（预留）
  - [ ] **变换栈**
    - [ ] `Save / Restore` — 保存/恢复画布状态
    - [ ] `Translate / Rotate / Scale` — 坐标变换
    - [ ] `SetClipRect` — 裁剪矩形
  - [ ] **CustomWidget 基类**
    - [ ] 继承 Widget，重写 `OnPaint(Canvas& canvas)`
    - [ ] 框架在 Draw 阶段自动创建 Canvas 传入
    - [ ] 用户自由绘制任意图形
  - [ ] **脚本层自绘**
    - [ ] Lua/JS 中可调用 Canvas API
    - [ ] 脚本定义的自绘控件（无需 C++ 编译）
  - [ ] **Canvas 后端**
    - [ ] BatchRenderer2D 后端（当前渲染器直接驱动）
    - [ ] RHI 后端（未来迁移到 RHI 抽象后）
    - [ ] 离屏渲染（Framebuffer Object → 纹理，用于缓存/特效）
- [ ] **手势系统**
  - [ ] 滑动（Swipe）
  - [ ] 长按（LongPress）
  - [ ] 双击（DoubleTap）
  - [ ] 捏合缩放（Pinch）
- [ ] **中文/阿拉伯文渲染修复**
  - [ ] 排查 stbtt 字形加载问题
  - [ ] 阿拉伯文字形连接（shaping，可能需要 HarfBuzz）

---

## Layer 5: HMI Runtime — 运行时引擎

加载和执行 HMI 项目的运行时。

### 待实现
- [ ] **项目格式定义**
  - [ ] 项目描述文件（project.json：入口场景、资源清单、配置）
  - [ ] 场景描述文件（scene.json：控件树 + 属性 + 绑定 + 动画）
  - [ ] 资源清单（manifest：所有资源的路径和元数据）
- [ ] **HMIRuntime 加载器**
  - [ ] 解析项目描述 → 加载资源 → 构建场景
  - [ ] 场景序列化/反序列化（JSON → Widget 树）
  - [ ] 属性绑定表达式解析（`speed.value → gauge.value`）
- [ ] **数据连接层**
  - [ ] DataModel 抽象（key-value 数据模型）
  - [ ] 信号源适配器（CAN/SOME-IP/DDS/MQTT → DataModel）
  - [ ] 模拟数据源（用于开发/测试）
  - [ ] DataModel → Widget 属性自动绑定
- [ ] **状态机**
  - [ ] 页面导航状态机
  - [ ] 控件状态机（如多媒体播放状态）
  - [ ] 条件跳转、守卫条件

---

## Layer 6: ScriptEngine — 脚本引擎

业务逻辑用脚本编写，支持热更新。

### 待实现
- [ ] **脚本语言选型**
  - [ ] Lua（轻量、嵌入式首选、sol2 绑定库）
  - [ ] JavaScript（V8/QuickJS，前端人员友好）
  - [ ] 或自定义 DSL
- [ ] **API 绑定**
  - [ ] 通过属性系统暴露 Widget 接口（`widget:get("value")`）
  - [ ] 事件监听（`widget:on("click", function() ... end)`）
  - [ ] 动画创建（`animate(widget, "opacity", 0, 1, 0.3)`）
  - [ ] 场景导航（`scene:push("settings")`）
  - [ ] 数据模型访问（`data:get("vehicle.speed")`）
  - [ ] 定时器（`setTimeout(fn, 1000)`）
- [ ] **脚本生命周期**
  - [ ] 场景脚本：OnEnter/OnExit/OnUpdate
  - [ ] 控件脚本：事件处理器
  - [ ] 全局脚本：应用级逻辑
- [ ] **热重载** — 修改脚本文件后自动重新执行
- [ ] **调试支持** — 断点/单步/变量查看（远程调试协议）

---

## Layer 7: ResourcePipeline — 资源管线

管理所有 UI 资产的生产、索引和打包。

### 待实现
- [ ] **资源类型**
  - [ ] 纹理：PNG/JPG → 压缩格式（KTX2/ASTC）+ 图集打包
  - [ ] 字体：TTF/OTF → 预烘焙字形集 或 运行时动态缓存
  - [ ] 样式：JSON 主题文件
  - [ ] 国际化：JSON 语言包
  - [ ] 3D 模型：OBJ/glTF → 优化格式
  - [ ] 动画：关键帧数据
  - [ ] 音频：WAV/OGG（预留）
  - [ ] UI 布局：JSON/XML 场景描述
  - [ ] 脚本：Lua/JS 源文件
- [ ] **图集打包器（Texture Atlas Packer）**
  - [ ] 将多张小图合并成大图集
  - [ ] 生成 UV 映射表
  - [ ] 支持 9-patch 标记
- [ ] **资源索引**
  - [ ] 统一 URI 寻址（`res://textures/btn_bg.png`）
  - [ ] 资源依赖图
  - [ ] 版本管理 / 增量更新
- [ ] **打包格式**
  - [ ] 开发模式：散文件 + 文件监听热重载
  - [ ] 发布模式：二进制资源包（.hmi/.pak）
  - [ ] 压缩 + 校验

---

## Layer 8: HMI Studio — IDE 工具

可视化开发工具，面向 HMI 设计师和开发者。

### 待实现
- [ ] **UI 编辑器**
  - [ ] 可视化拖拽控件到画布
  - [ ] 属性面板（实时编辑控件属性）
  - [ ] 控件树大纲视图
  - [ ] 布局辅助线 / 对齐吸附
  - [ ] 撤销/重做（Undo/Redo）
  - [ ] 多场景/多页面管理
- [ ] **样式编辑器**
  - [ ] 可视化主题配色
  - [ ] 实时预览
  - [ ] 主题导入/导出
- [ ] **动画编辑器**
  - [ ] 时间轴（Timeline）关键帧编辑
  - [ ] Easing 曲线可视化
  - [ ] 预览播放
- [ ] **脚本编辑器**
  - [ ] 语法高亮 + 自动补全
  - [ ] 内联文档（API 提示）
  - [ ] 错误标注
- [ ] **仿真模拟器**
  - [ ] 在 PC 上模拟目标屏幕分辨率
  - [ ] 模拟触屏/旋钮/按键输入
  - [ ] CAN 信号模拟面板
  - [ ] 多屏幕预览（仪表盘 + 中控）
- [ ] **资源管理器面板**
  - [ ] 资源浏览/导入/导出
  - [ ] 图片预览 / 字体预览
  - [ ] 资源引用查找
- [ ] **构建/部署**
  - [ ] 一键打包资源
  - [ ] 交叉编译配置
  - [ ] 目标设备部署（SSH/ADB）
  - [ ] 远程调试连接

---

## 迭代策略建议

不建议一次性铺开所有层，按以下阶段递进：

### 阶段 A：夯实基础（当前 → 近期）
1. 修复已知 bug（字体渲染、主题切换）
2. ResourceManager 基础版（统一资源加载 + 引用计数）
3. 渲染增强（圆角矩形、裁剪、控件贴图）
4. 更多基础控件（ScrollView、TextInput、ImageView）

### 阶段 B：运行时引擎（中期）
5. 场景序列化/反序列化（JSON → Widget 树）
6. 脚本引擎集成（Lua + sol2）
7. DataModel + 信号模拟
8. RHI 抽象 + OpenGL ES 后端

### 阶段 C：工具链（中后期）
9. 资源管线（图集打包、资源包格式）
10. HMI Studio 编辑器（基于 ImGui 的原型）
11. 仿真模拟器

### 阶段 D：平台扩展（后期）
12. Win32 原生后端
13. Wayland 后端
14. Vulkan 后端
15. 嵌入式目标部署

---

## 参考项目

| 项目 | 定位 | 可借鉴 |
|------|------|--------|
| Qt/QML | 通用 GUI 框架 | 属性系统、QML 声明式 UI、信号/槽 |
| EB GUIDE | 车机 HMI 工具 | 状态机驱动、模型/视图分离、IDE |
| Kanzi | 车机 3D HMI | RHI 抽象、Lua 脚本、资源管线 |
| Flutter | 跨平台 UI | Skia 渲染、Widget 树、声明式 UI |
| LVGL | 嵌入式 GUI | 轻量控件、多后端渲染 |
| Dear ImGui | 即时模式 GUI | 工具类 UI 参考（我们的调试面板） |

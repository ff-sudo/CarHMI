# M1.9 布局增强（Margin/Padding + 百分比尺寸）— 经验总结

## 实现方案

### 核心数据结构

**Insets**：四边独立值，多种构造方式满足不同场景：
- `Insets(10)` — 四边相等
- `Insets(10, 20)` — 上下/左右
- `Insets(10, 20, 10, 20)` — 四边独立

**SizePolicy**：宽高各自独立的 Fixed/Percent/Fill 模式。
- Fixed：用 Widget 的 m_size（默认，向后兼容）
- Percent：占父容器可用空间的百分比
- Fill：平分剩余空间

### 双轮 Recalculate 算法

**第一轮（测量）**：统计 Fixed 子控件主轴占用，统计 Percent 总比例和 Fill 计数。

**第二轮（分配尺寸）**：
1. Percent 基于**去掉 Fixed 后的剩余空间**计算（不是整个 avail），且 totalPercent > 100% 时自动按比例缩放
2. Fill 平分 Percent 之后的剩余空间
3. 交叉轴的 Fill/Percent 独立处理

**第三轮（放置位置）**：cursor 逐个放置

> **关键修复**：初版 Percent 基于整个 availW 计算，Fixed+Percent 会超出容器。改为基于 remainAfterFixed 后不再溢出。

### ImGui 调试面板模块化

所有 ImGui 调试函数放在 `gui/debug/debug_panel.h`，header-only inline 函数。
Scene 按需组合调用，不再每个 Scene 重复写 ImGui 代码。

## 踩过的坑

### 1. 向后兼容：float → Insets

BoxLayout 构造函数原来是 `float padding`，改为 `Insets m_padding` 后，构造函数参数保持 `float`，内部转为 `Insets(padding)`。所有已有代码（`BoxLayout(id, pos, size, dir, 20, 12)`）无需任何改动。

**教训**：改内部数据结构时，公共 API 保持兼容是第一优先级。

### 2. Fill 计算要减去 margin

Fill 分配剩余空间时，每个 Fill 子控件的实际内容尺寸 = fillSize - margin，不是直接用 fillSize。否则 margin 会被"吃掉"，控件看起来会溢出。

### 3. 交叉轴 Fill 的处理

Vertical 布局中，widthMode=Fill 的子控件要填满 availW（扣除自身 margin）。这是独立于主轴 Fill 逻辑的。不要在第一轮只处理主轴。

### 4. Recalculate 调用时机

`Recalculate()` 在以下时机被调用：
- `AddChild()` 后
- `SetPadding()` / `SetSpacing()` / `SetDirection()` 后

但**子控件的 margin/sizePolicy 改变后不会自动触发**父容器 Recalculate。这是有意设计——避免在构建阶段频繁重算。Demo 中 ImGui 修改后手动调 `Recalculate()`。

后续如果需要自动响应，可以通过属性系统的 OnChanged 回调实现。

### 5. ImGui inline 函数的编译

DebugPanel 是 header-only（inline），被多个 .cpp include 时不会链接冲突。但注意所有函数必须标 `inline`，否则会产生多重定义错误。

### 6. OnImGui 中调 SceneManager::Pop 导致 use-after-free

`SceneManager::Update()` 先调 `OnUpdate()`，再调 `OnImGui()`。如果在 `OnImGui()` 中直接调 `Pop()`，当前 Scene 的 `OnExit()` 会 `delete m_root`，但 `OnImGui()` 栈帧还在执行，后续代码访问已释放成员就崩溃。

**修复**：`Pop()` 改为设标记（`m_pendingPop`），在 `Update()` 末尾（OnImGui 返回后）才真正执行 Pop。这个修复是全局性的，所有 Scene 都受益。

### 7. Percent 溢出的根因

初版 Percent 基于整个 available 空间计算。一个 80% width 的子控件 + 一个 200px fixed 子控件在水平布局中：80% × 460 + 200 = 568 > 460，直接溢出。

**修复**：Percent 的基数改为 `remainAfterFixed`（去掉 Fixed 子控件和 spacing 后的剩余）。同时 `totalPercent > 100%` 时自动按比例缩放，确保多个 Percent 子控件不会互相挤爆。

## 架构决策记录

| 决策 | 选择 | 理由 |
|------|------|------|
| Margin 放在 Widget 基类 | 是 | 任何控件都可能需要外边距 |
| SizePolicy 放在 Widget 基类 | 是 | 布局计算需要统一访问 |
| Insets 独立文件 | 是 | Margin 和 Padding 都用，避免循环依赖 |
| DebugPanel header-only | 是 | 避免给 GUI 库增加 ImGui 依赖，只在 Gallery 中被 include |
| 不自动触发父级 Recalculate | 是 | 避免构建阶段级联重算，后续可通过事件系统扩展 |

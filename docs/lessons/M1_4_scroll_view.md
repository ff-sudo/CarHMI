# M1.4 ScrollView 控件 — 经验总结

## 实现方案

### 核心设计

ScrollView 是通用滚动容器，支持垂直+水平双向滚动。关键组成：
- **scrollX / scrollY**：两个独立的滚动偏移
- **Scissor 裁剪**：PushClipRect 限制子控件绘制范围
- **子控件位移**：Draw/Update 时临时修改子控件 pos（加上 -scrollOffset），用完恢复
- **滚动条**：track + thumb，支持鼠标拖拽

### 滚动条拖拽

通过订阅 MouseButtonEvent + MouseMoveEvent 实现：
1. MouseButton pressed → 检测点击是否在 thumb 上（HitTest）
2. 记录 dragStartMouseY/X 和 dragStartScroll
3. MouseMove → 计算鼠标位移 / 可移动范围 × maxScroll = 新 offset
4. MouseButton released → 停止拖拽

### 内容区与滚动条分离

裁剪区域在有滚动条时缩小：
```
clipW = size.x - (hasVScroll ? barWidth + margin*2 : 0)
clipH = size.y - (hasHScroll ? barWidth + margin*2 : 0)
```
滚动条绘制在裁剪区域外，叠在最上层。

## 踩过的坑

### 1. 子控件位移的临时修改模式

Draw 时需要让子控件"看起来"在偏移后的位置绘制。由于 GetAbsolutePos 是递归的（parent->GetAbsolutePos() + m_pos），不能简单地修改 ScrollView 自身的位置。

方案：临时修改每个 child 的 pos，Draw 后恢复。Update 也要同样处理，否则鼠标 hit test 会对不上。

```cpp
glm::vec2 origPos = child->GetPos();
child->SetPos(origPos + glm::vec2(-scrollX, -scrollY));
child->Draw(ctx);
child->SetPos(origPos);
```

这个模式虽然简单粗暴，但在单线程渲染循环中完全安全。

### 2. 鼠标事件的全局订阅 vs 局部响应

ScrollView 通过 EventBus 全局订阅 MouseScrollEvent/MouseButtonEvent/MouseMoveEvent。但只在 IsHovering() 或 m_dragging 时才响应。

注意：多个 ScrollView 同时存在时，都会收到事件。IsHovering() 检查确保只有鼠标下方的 ScrollView 响应滚轮。拖拽状态（m_draggingV/H）确保拖拽开始后即使鼠标移出区域也继续跟踪。

### 3. 内容区与滚动条重叠

初版没有给滚动条预留空间，内容和滚动条重叠。修复：裁剪区域缩小 barWidth + margin*2，给滚动条让出空间。

### 4. 自动内容尺寸计算

RecalcContentSize() 遍历所有子控件，取 max(pos + size + margin)。这意味着子控件必须在 AddChild 时已经有正确的 pos 和 size。如果后续动态修改子控件位置，需要手动调 RecalcContentSize() 或 SetContentSize()。

## 架构决策记录

| 决策 | 选择 | 理由 |
|------|------|------|
| 双向滚动 | 是 | 车机 HMI 地图/表格等场景需要水平滚动 |
| 滚动条拖拽 | EventBus 订阅 | 不需要改 Widget 基类或 UIContext |
| 子控件位移方式 | 临时修改 pos | 最简方案，不需要引入变换矩阵 |
| 裁剪方式 | Scissor (PushClipRect) | 利用已有基础设施，GPU 硬件裁剪 |
| 内容区缩小 | 有滚动条时减去 barWidth | 避免内容和滚动条视觉重叠 |

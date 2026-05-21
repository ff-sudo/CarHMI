# M1.10 Canvas 自绘 API 基础版 — 经验总结

## 实现方案

### Canvas 定位

Canvas 是**轻量级绘图上下文包装器**，不是 Widget 基类。在 Widget::Draw() 中临时构造，用完即弃，不持有跨帧状态。

设计理念：所有坐标相对于 Canvas 原点（通常是 Widget 的绝对位置），通过 `P(x, y)` 内部转换为屏幕坐标。

### API 分层

```
用户代码 → Canvas（高层 API）→ BatchRenderer2D（低层原语）→ OpenGL
```

Canvas 的每个方法都是 1-N 个 BatchRenderer2D 调用的薄封装：
- FillRect → DrawQuad
- StrokeRect → 4 个 DrawQuad（四条边）
- FillRoundRect → DrawRoundedRect
- StrokeCircle → DrawRing
- FillPath → N 个 DrawTriangle（triangle fan）

### CanvasWidget 用 callback 而非继承

```cpp
canvasWidget->SetOnDraw([](Canvas& cv) {
    cv.FillCircle(50, 50, 30);
});
```

比定义子类更灵活，Scene 中可以直接用 lambda。

## 踩过的坑

### 1. FillPath 需要 DrawTriangle

Canvas 的 FillPath 需要绘制任意三角形，但 BatchRenderer2D 原来没有这个原语。所有形状都是基于 quad（4 顶点 + 6 索引）。

解决：新增 `DrawTriangle(a, b, c, color)` 方法，使用退化 quad（v3=v0），索引模式 (0,1,2,2,3,0) 生成两个三角形：(a,b,c) 是有效的，(c,a,a) 是退化的（面积为零，不可见）。这正是 DrawCircle 内部用的同一种技巧。

### 2. FillPath 只支持凸多边形

Triangle fan 从 pathPoints[0] 出发，连接后续每对相邻点。对凸多边形完美工作，但**凹多边形会有填充错误**。

这是 v1 的已知限制。完整的多边形三角剖分（ear clipping 等）留给后续版本。实际 HMI 使用中，大部分自定义形状（仪表盘指针、按钮形状等）都是凸的或可以拆分为凸部分。

### 3. StrokeRect 的角连接

StrokeRect 用 4 个 DrawQuad 画四条边。在角上会有微小的重叠（两条边的端点重合区域被画了两次），如果颜色有半透明度（alpha < 1），角上会看到颜色加深。

对于 v1 这是可接受的。完美方案需要 miter/bevel/round 角连接处理，这属于完整的路径描边引擎范畴。

### 4. ClipRect 的 depth 跟踪

Canvas 内部用 `m_clipDepth` 计数器跟踪推入了多少层 scissor，`ResetClip()` 时全部弹出。这比让用户手动 PopScissor 安全——避免忘记弹出导致后续绘制被错误裁剪。

### 5. Save/Restore 只保存自有状态

Canvas 的 Save/Restore 保存 offset + fillColor + strokeColor + strokeWidth。不保存 scissor 状态（scissor 由 BatchRenderer2D 独立管理）。如果在 Save 和 Restore 之间 ClipRect，需要在 Restore 前手动 ResetClip。

## 架构决策记录

| 决策 | 选择 | 理由 |
|------|------|------|
| Canvas 是独立类而非 Widget 基类 | 是 | 任何 Widget 都可以在 Draw() 中用 Canvas，不需要继承关系 |
| CanvasWidget 用 callback | 是 | 比继承灵活，Scene 里直接用 lambda |
| v1 不做旋转/缩放 | 是 | 需要改顶点变换方式，复杂度大，留给后续 |
| v1 不做贝塞尔曲线 | 是 | 需要德卡斯特里奥细分，单独的功能点 |
| FillPath 用 triangle fan | 是 | 简单够用，凸多边形覆盖大部分场景 |
| 新增 DrawTriangle 到 RHI 层 | 是 | 三角形是基本图元，Canvas 和后续 mesh 渲染都需要 |

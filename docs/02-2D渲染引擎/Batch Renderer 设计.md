# Batch Renderer 设计

## 为什么要批量渲染？

OpenGL 每次 `glDrawElements` 调用（即一个 draw call）都有 CPU-GPU 通信开销。如果逐个绘制每个矩形，1000 个矩形就是 1000 次 draw call — 性能极差。

**Batch Renderer 核心思想**：把多个图元的顶点数据收集到一个大 VBO 中，一次 draw call 提交。

```
传统方式：1 quad = 1 draw call    →  1000 quads = 1000 draw calls
批量方式：N quads = 1 draw call   →  1000 quads = 1 draw call
```

## 顶点布局

```cpp
struct Vertex {
    glm::vec2 position;   // 屏幕坐标
    glm::vec4 color;      // RGBA 颜色
    glm::vec2 texCoord;   // 纹理 UV
    float texIndex;       // 纹理槽索引 (0=白色默认)
};
```

每个四边形 = 4 个顶点 + 6 个索引（两个三角形）

```
索引模式: 0,1,2, 2,3,0
   3 ---- 2
   |    / |
   |  /   |
   0 ---- 1
```

## 多纹理支持

OpenGL 3.3 支持至少 16 个纹理单元同时绑定。Shader 中用 `sampler2D u_Textures[16]` 数组采样。

- 槽 0 固定绑定 1x1 白色纹理（用于纯色渲染）
- 每帧遇到新纹理就分配一个槽
- 16 个槽满时触发 flush（提交当前批次，开启新批次）

## Flush 触发条件

1. 顶点满（10000 个四边形 = 40000 顶点）
2. 纹理槽满（16 个）
3. 帧结束调用 `End()`

## 关键代码路径

```
src/renderer/batch_renderer2d.h   — 类声明、Stats 结构
src/renderer/batch_renderer2d.cpp — 完整实现
resources/shaders/batch2d.vert    — 顶点着色器
resources/shaders/batch2d.frag    — 片段着色器（switch-case 纹理采样）
```

## 性能监控

通过 `GetStats()` 获取每帧的渲染统计，显示在 [[../00-环境搭建/环境搭建|ImGui 调试面板]] 中：
- Draw Calls 数量
- 四边形数量
- 纹理槽使用数

## 扩展图元

除了四边形，BatchRenderer2D 还支持以下图元（用于车机控件 Gauge/Knob 等）：

### DrawCircle
用三角形扇形近似圆形，每个三角形共享圆心顶点。

```cpp
renderer.DrawCircle(center, radius, color, segments);
```

### DrawArc / DrawRing
圆弧/圆环，用内外两个半径的四边形条带近似。

```cpp
renderer.DrawArc(center, innerR, outerR, startAngle, endAngle, color, segments);
renderer.DrawRing(center, innerR, outerR, color, segments);  // 完整圆环
```

角度单位为度数。用于 Gauge 仪表盘的弧形轨道和填充。

### DrawLine
用矩形模拟线段，沿法线方向扩展厚度。

```cpp
renderer.DrawLine(from, to, thickness, color);
```

用于 Gauge 指针、Knob 指示线等。

### DrawSubTexturedQuad
带自定义 UV 坐标的纹理四边形，用于字体渲染（从图集中取子区域）。

```cpp
renderer.DrawSubTexturedQuad(pos, size, texture, uvMin, uvMax, tint);
```

## 相关笔记

- [[Shader 编写笔记]]
- [[纹理管理]]
- [[../00-环境搭建/环境搭建]]

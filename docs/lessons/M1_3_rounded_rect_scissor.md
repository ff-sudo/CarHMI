# M1.3 圆角矩形 + Scissor 裁剪 — 经验总结

## 实现方案

### 圆角矩形：CPU 几何细分（9-patch 方案）

将圆角矩形分解为：
- 1 个中心矩形
- 4 个边缘矩形（上/下/左/右）
- 4 个角扇形（每角 N 个三角形段，默认 N=8）

总计 5 + 4×8 = 37 个 quad/圆角矩形。

**为什么不用 SDF shader**：
- 不需要改顶点格式和 shader，与现有 batch 完全兼容
- 车机场景圆角矩形数量有限（几十个级别），CPU 开销可忽略
- 后续 M4 做 RHI 抽象时再考虑 SDF 方案

### Scissor 裁剪栈

- 用 `std::vector<glm::vec4>` 维护栈
- 每次 Push 与栈顶取交集（支持嵌套）
- Push/Pop 前先 Flush 当前 batch（因为同一 batch 内不能换 scissor）
- 栈空时 disable GL_SCISSOR_TEST

## 踩过的坑

### 1. Scissor Y 轴翻转

OpenGL 的 `glScissor` Y=0 在窗口**底部**，但我们的正交投影是 Y 向下（Y=0 在顶部）。

正确转换：`glScissor(x, viewportHeight - y - h, w, h)`

忘了这个转换的话，裁剪区域会出现在"镜像"位置。

### 2. 圆角半径需要 clamp

如果 radius > min(width, height)/2，四个角的弧线会交叉产生自交几何。

解决：开头 `radius = std::min(radius, std::min(size.x, size.y) * 0.5f)`

### 3. 退化情况处理

radius < 0.5px 时直接 fallback 到 DrawQuad，避免无意义的扇形三角生成。

### 4. Batch 与 Scissor 的交互

关键认识：**glScissor 是 per-draw-call 级别的状态**，不能在已经 batch 好的顶点数据中间切换。

所以 PushScissor/PopScissor 必须先 Flush()。这意味着每次 scissor 变化会打断 batch，产生一个额外 draw call。这在实际使用中可接受（一个 ScrollView 最多 +2 draw calls）。

### 5. CLI 构建环境（MSVC 在 bash 中）

MSVC 编译需要 INCLUDE/LIB 环境变量指向标准库和 SDK 头文件。CMake configure 阶段的 preset `environment` 字段只在 configure 时生效，**不会传递给 build 阶段的 Ninja 进程**。

解决方案：
- 在 bash 中构建时需要手动 export INCLUDE/LIB/PATH
- `CMAKE_RC_COMPILER` 需要在 cacheVariables 中指定完整路径（不能只靠 PATH）

后续可以写一个 `build.sh` 脚本封装这些环境变量。

## 架构决策记录

| 决策 | 选择 | 理由 |
|------|------|------|
| 圆角实现方式 | CPU 几何细分 | 不改 shader/顶点格式，与现有 batch 兼容 |
| 角细分数 | 默认 8 段/角 | 视觉足够平滑，quad 数可控 |
| Scissor 管理层级 | 放在 BatchRenderer2D | 与 Flush 紧耦合，放在更上层会泄漏实现 |
| GUI 层暴露方式 | UIContext 转发 | 保持 widget 不直接操作 renderer |

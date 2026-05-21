# M1.8 定时器/调度器 — 经验总结

## 实现方案

### TimerManager 设计

Meyer 单例模式（与 AnimationManager/EventBus 一致），header-only。

核心数据结构：`std::vector<TimerEntry>`，每个 entry 包含：
- id（用于 Connection 取消）
- callback（std::function<void()>）
- intervalMs（int 毫秒）
- elapsedMs（float 累加）
- repeating（bool）
- cancelled（延迟删除标记）

### Update 循环

向后遍历 + 标记删除模式（与 AnimationManager 一致）：
1. 已 cancelled 的 entry 立即 erase
2. elapsedMs += dt * 1000
3. elapsed >= interval 时触发 callback
4. repeating=true 则 elapsed -= interval（保持漂移补偿）
5. repeating=false 则 erase

### Connection RAII 取消

`SetTimeout/SetInterval` 返回 `Connection`，其 disconnect 函数捕获 `[this, id]` 标记 `cancelled=true`。Scene 的 `ConnectionGroup` 析构时自动取消所有定时器。

## 踩过的坑

### 1. 时间单位选择：毫秒 vs 秒

主循环 dt 是秒（float），但用户 API 用毫秒更直观（`SetTimeout(cb, 2000)` 比 `SetTimeout(cb, 2.0f)` 更清晰）。

内部用 `float elapsedMs` 累加 `dt * 1000.0f`，比较时与 `int intervalMs` 比较。不要用 int 累加 dt 转换后的值——每帧约 16.67ms，int 截断为 16 会累积误差。

### 2. repeating 的漂移补偿

Interval 触发后 `elapsed -= interval` 而不是 `elapsed = 0`。如果一帧 dt 很大（如窗口拖动卡顿 200ms），interval=100ms 时会在一帧内触发 2 次，确保频率稳定。

### 3. 回调中创建新定时器的安全性

向后遍历确保新 push_back 的 entry 不会在当前帧被遍历到（新 entry 在 vector 尾部，向后遍历从尾部开始，但 i 已经过了那个位置）。

但需注意：回调中如果调用 `ClearAll()`，当前遍历的 vector 被清空，后续迭代会越界。当前设计中 `ClearAll()` 是暴力清空，回调中调它会有问题。实际使用中不应在回调内 ClearAll，如果需要可以改为标记延迟清空。

### 4. 这个模块极其简单

整个 TimerManager 约 80 行代码，header-only，无外部依赖。这证明了好的架构基础设施（Connection RAII + 单例模式 + frame dt 驱动）让新功能的实现非常轻量。

## 架构决策记录

| 决策 | 选择 | 理由 |
|------|------|------|
| API 时间单位 | int 毫秒 | 用户直觉（2000 = 2秒），避免浮点字面量 |
| 内部累加 | float 毫秒 | 避免 int 截断 dt 的精度损失 |
| 存储方式 | vector + 标记删除 | 简单，定时器数量少（<100）性能无压力 |
| 返回类型 | Connection | 复用现有 RAII 模式，Scene 退出自动取消 |
| 放置位置 | core/timer/ 独立子目录 | 与 animation/property/scene 平级，职责清晰 |

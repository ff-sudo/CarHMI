# Retained Mode vs Immediate Mode

## 两种 GUI 范式

### Immediate Mode（即时模式）
代表：**ImGui**

每帧重新声明整个 UI，没有持久状态：
```cpp
// 每帧都执行
if (ImGui::Button("Click")) {
    // 处理点击
}
ImGui::SliderFloat("Speed", &speed, 0, 240);
```

优点：代码简单，无状态同步问题，非常适合调试工具
缺点：难以做复杂布局和动画，每帧重建开销

### Retained Mode（保留模式）
代表：**Qt, Android View, 我们的 GUI 库**

控件树持久存在，状态保留在对象中：
```cpp
// 只在初始化时构建
auto* btn = new Button(1, {20, 100}, {200, 40}, "Click");
btn->SetOnClick([&]() { /* 处理 */ });
panel->AddChild(btn);

// 每帧只需 Update + Draw
panel->Update(ctx);
panel->Draw(ctx);
```

优点：适合固定布局、状态驱动的界面（如车机 HMI）
缺点：需要管理控件生命周期和状态同步

## 车机 HMI 为什么选 Retained Mode

1. 界面布局相对固定，不会每帧变化
2. 需要复杂动画和过渡效果（retained 更容易实现）
3. 状态驱动：CAN 信号 → 控件属性变化
4. 性能可控：只更新变化的部分

## 我们的方案：两者结合

- **Retained Mode GUI**：主界面（仪表盘、控制面板）
- **ImGui**：调试面板（FPS、渲染统计、CAN 信号模拟）

## 相关笔记

- [[事件分发机制]]
- [[控件树设计]]
- [[../02-2D渲染引擎/Batch Renderer 设计]]

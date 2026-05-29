# M3 脚本引擎 — 技术调研方案

> 调研日期：2026-05-29
> 状态：方案确定，待实施

---

## 技术选型

| 决策 | 选择 | 理由 |
|------|------|------|
| 绑定库 | **sol2 v3.3.0** | C++17、header-only、Property 绑定天然适配、4.2k Stars、最成熟 |
| Lua 版本 | **Lua 5.4.7** | 分代 GC（减少卡顿）、原生整数、`_ENV` 沙箱支持 |
| 备选 | LuaBridge3 | 若 sol2 编译时间不可接受时降级 |

**淘汰方案**：
- Selene / Kaguya — 已停维（2018/2019），不考虑
- MicroPython — 太慢，GC 停顿明显
- 纯 Lua C API — 18 种 Widget 手动绑定工作量不可接受
- JavaScript (V8/QuickJS) — 重量级，GC 问题更严重

---

## 行业调研

### Qt/QML (JavaScript V4)
- **优点**：声明式 + 命令式混合、响应式属性绑定、工具链成熟
- **痛点**：GC 卡顿、隐式作用域导致 bug、类型不安全、调试困难
- **借鉴**：响应式属性模型（我们的 Property<T> 已实现）

### Unreal Engine (Blueprint + UnLua)
- **优点**：反射系统自动生成绑定、协程做异步逻辑
- **借鉴**：从现有类型系统自动导出 Lua 绑定，减少手工维护

### Godot (GDScript)
- **关键洞察**：专为引擎设计的脚本语言 > 通用语言硬塞
- **借鉴**：Lua API 应贴合 CarHMI 的概念（widget/property/event/scene），不做裸 C++ 反射

### LVGL + MicroPython
- **优点**：从 C 头文件自动生成绑定
- **痛点**：慢、GC 卡顿

### 车载 HMI 工具 (EB GUIDE / Kanzi / Altia)
- EB GUIDE：状态机 + C++ 插件，无通用脚本
- Kanzi：JS (Duktape) + 属性系统
- Altia：纯 C 生成，无运行时脚本
- **结论**：非安全域信息娱乐 HMI 用 Lua 是合适的，灵活性远超状态机

---

## 架构设计

### 核心理念：Scene 驱动脚本生命周期

```
JsonScene 加载 → 创建 Lua 沙箱环境 → 执行脚本 → 绑定控件
Scene Pop → 断开回调 → 销毁 Lua 环境
```

### 层次结构

```
┌─────────────────────────────────────────┐
│  Lua Script (业务逻辑)                    │
│  onEnter() / onUpdate(dt) / onExit()     │
├─────────────────────────────────────────┤
│  Lua API Layer (安全暴露)                 │
│  ui.find(id) / widget:set() / timer()    │
├─────────────────────────────────────────┤
│  ScriptEngine (sol2 封装)                 │
│  加载/沙箱/热重载/错误隔离                │
├─────────────────────────────────────────┤
│  C++ Runtime (Widget/Property/Event/Anim)│
└─────────────────────────────────────────┘
```

### 控件暴露策略：Handle/ID，非裸指针

```lua
-- 安全：通过 ID 查找，内部检查有效性
local btn = ui.find(3)
btn:setText("Hello")
btn:on("clicked", function() print("clicked!") end)

-- 若控件已销毁，操作变为 no-op + warn 日志（不崩溃）
```

**理由**：避免 QML 的经典痛点（JS 持有 C++ 裸指针 → 悬垂引用崩溃）

### 脚本生命周期 Hooks

```lua
-- scene_script.lua
function onEnter()
    local slider = ui.find(20)
    slider:on("changed", function(val)
        ui.find(3):setText(string.format("Speed: %.0f", val))
    end)
end

function onUpdate(dt)
    -- 每帧逻辑
end

function onExit()
    -- 自动清理，也可手动
end
```

### 热重载机制

1. 文件变化检测（复用 120 帧检查模式）
2. 调用 `onSaveState()` → 返回需要保持的状态表
3. 清除 `package.loaded`，重新 require
4. 调用 `onRestoreState(savedState)`
5. 重新绑定回调

**关键**：行为（函数）和状态（数据）分离

---

## 避免行业痛点

| 痛点 | 来源 | 方案 |
|------|------|------|
| GC 卡顿 | QML/JS | Lua 5.4 分代 GC + 每帧预算 `lua_gc(LUA_GCINC, 1ms)` |
| 悬垂引用 | QML C++/JS 混合 | Handle 系统 + Widget 销毁时自动 invalidate |
| 热重载丢状态 | 通用问题 | `onSaveState/onRestoreState` 协议 |
| 类型不安全 | 动态语言通病 | sol2 `protected_function` + 参数类型检查 |
| 调试困难 | Lua 生态 | lua-debug VSCode DAP + 应用内错误 Overlay |
| 线程安全 | lua_State 非线程安全 | 全部主线程，跨线程用 `PostDeferred` |
| 性能陷阱 | 频繁跨界调用 | 批量操作 API，缓存 local 引用 |
| 脚本错误拖垮系统 | 生产环境 | 每脚本独立 pcall，出错则 disable + 降级 |

---

## API 设计

```lua
-- Widget 操作
local w = ui.find(id)         -- 按 ID 查找
w:get("value")                -- 读属性
w:set("text", "Hello")        -- 写属性
w:on("clicked", fn)           -- 事件监听（返回 connection）
w:setVisible(true)

-- 定时器（映射 TimerManager）
local c = timer.setTimeout(fn, 2000)
local c = timer.setInterval(fn, 500)
c:cancel()

-- 动画
anim.tweenTo(widget, "value", 100, 0.5, "outQuad")

-- 场景导航
scene.push("scenes/next.json")
scene.pop()

-- 数据模型
local model = data.get("vehicle")
model:set("speed", 60)
model:on("speed", function(old, new) ... end)

-- 日志
log.info("message")
log.warn("message")
```

---

## 沙箱安全

```lua
-- 每个脚本环境只暴露白名单
local sandbox = {
    ui = ui_module,
    timer = timer_module,
    anim = anim_module,
    scene = scene_module,
    data = data_module,
    log = log_module,
    -- 标准库（安全子集）
    math = math, string = string, table = table,
    pairs = pairs, ipairs = ipairs, tostring = tostring,
    -- 禁止：os, io, debug, loadfile, dofile
}
```

---

## CMake 集成方案

```cmake
# Lua 5.4 源码编译（~30 个 .c 文件，秒级构建）
FetchContent_Declare(lua
    URL https://www.lua.org/ftp/lua-5.4.7.tar.gz)

# sol2 header-only
FetchContent_Declare(sol2
    GIT_REPOSITORY https://github.com/ThePhD/sol2.git
    GIT_TAG v3.3.0)
```

---

## 实现计划

| 阶段 | 内容 | 新增文件 |
|------|------|----------|
| 3.1 | Lua+sol2 集成，ScriptEngine 基础类 | `modules/script/include/script/script_engine.h`, `src/script_engine.cpp`, CMake |
| 3.2 | Widget/Property/Timer/Anim API 绑定 | `src/lua_ui_bindings.cpp`, `src/lua_timer_bindings.cpp` |
| 3.3 | ScriptedScene（Lua 驱动的 Scene 子类） | `include/script/scripted_scene.h`, `src/scripted_scene.cpp` |
| 3.4 | 热重载 + 错误隔离 | 内置于 ScriptEngine |
| 3.5 | Animation API 绑定 | `src/lua_anim_bindings.cpp` |

### 交付物
- Gallery 中 "Script" 按钮，加载 `resources/scripts/demo.lua`
- 修改 Lua 文件后 UI 实时变化
- 脚本错误显示在 DebugPanel，不崩溃

---

## 参考资料

- sol2 文档：https://sol2.readthedocs.io/
- Lua 5.4 参考手册：https://www.lua.org/manual/5.4/
- lua-debug VSCode：https://github.com/actboy168/lua-debug
- UnLua (UE Lua 集成参考)：https://github.com/Tencent/UnLua

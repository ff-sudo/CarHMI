# 贡献指南

## PR 流程

1. 基于 `develop_work` 分支创建 feature 分支
2. 开发完成后提交 — 提交信息用中文，简洁描述"做了什么"
3. 确保 `cmake --build build/dev-win` 编译通过
4. 运行测试：`cd build/dev-win && ctest --output-on-failure`
5. 全部通过后发起 PR 到 `develop_work`

## 编码规范

- C++17，命名空间 `CarHMI::<Module>`
- 缩进：4 空格，Allman 括号风格
- 头文件用 `#pragma once`
- 模块公共 API 通过 `modules/<name>/include/<name>/` 暴露
- `src/` 内部代码不可被外部模块访问

## 关键规则

- **依赖方向**：只能上层依赖下层（GUI→Core→PAL），禁止反向 include
- **事件订阅必须保存 Connection**：`EventBus::Subscribe()` 返回 RAII Connection，丢弃即断开
- **线程安全**：从非主线程写属性用 `SetFromThread()`，发事件用 `PostDeferred()`
- **新模块**使用 `cmake/CarHMIModule.cmake` 中的 `carhmi_add_module()` 宏

## 测试

```bash
cmake --build build/dev-win --target carhmi_core_tests carhmi_gui_tests
cd build/dev-win && ctest --output-on-failure
```

新增功能应补充测试用例。测试框架：GoogleTest v1.15。

## 发布流程

1. 里程碑完成后更新 `CHANGELOG.md`
2. 同步 `CMakeLists.txt` 版本号
3. 打 git tag：`git tag vX.Y.Z -m "vX.Y.Z: 描述"`
4. 标记 ROADMAP 对应任务为 ✅

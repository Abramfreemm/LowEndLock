# Task 03 — Sidechain 输入路由

- 状态：In Progress
- 依赖：Task 02

## 目标

让插件能接收 Kick 作为 sidechain 参考信号。

## 输出

- 启用 sidechain bus。
- `processBlock()` 中读取 sidechain buffer。
- UI 显示 sidechain 是否有信号。

## 验收标准

- DAW 能将 Kick 路由到 sidechain。
- 插件能检测 sidechain 活动状态。

## 进度记录

- 已在 `BusesProperties` 中添加 `Sidechain` 输入总线。
- 已更新 `isBusesLayoutSupported` 以允许额外 sidechain。
- `processBlock` 已分离主信号和 sidechain 缓冲。
- 已计算 sidechain RMS 并通过原子变量供 UI 读取。
- UI 已加入 `Kick Sidechain` 状态显示，定时刷新。
- `LowEndLock - Shared Code` 编译通过。
- 待用户在 DAW 中验证 sidechain 路由和状态显示。

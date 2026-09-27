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

## 根因结论（重要）

Logic Pro 的 AU sidechain 菜单不是靠「静态声明第二个输入总线」触发的，
而是靠「输入总线数量可写」（`BusCountWritable`）触发。Logic 会调用
`kAudioUnitProperty_ElementCount` 把输入总线从 1 加到 2，把它当作 sidechain。
JUCE 的 AU wrapper 把 `BusCountWritable` 映射为 `canAddBus()/canRemoveBus()` 返回 true。
两者默认都返回 false，所以插件即使静态声明了第二个输入总线，Logic 也不会显示侧链菜单。
这解释了「Logic 自带 Compressor 有 sidechain，而 LowEndLock 没有」。

## 修复方案

- 不再在构造函数里静态声明 `Sidechain` 总线，只保留主 `Input`（stereo）与 `Output`。
- 重写 `canAddBus()` / `canRemoveBus()`，允许输入总线动态增删（最多 2 个输入总线）。
- 重写 `canApplyBusCountChange()`，把动态新增的输入总线命名为 `Sidechain`、默认 stereo。
- 更新 `isBusesLayoutSupported()`，校验可选的 sidechain 总线允许 mono / stereo / disabled。
- `processBlock()` 现在用 `getBusCount(true) > 1` 安全判断 sidechain 是否存在，避免总线不存在时崩溃。
- 修复 JUCE 9.0.2 自身的 bug：`JuceAU::SetBusCount()` 同步了错误的 bus 布局标签，
  导致 Logic 加 sidechain 后 `AudioUnitInitialize` 返回 -10868。
  补丁见 `patches/juce-9.0.2-logic-sidechain-buscount.patch`。

## 进度记录

- AU 校验（`auval -v aufx Vtqg Manu`）通过，默认 1 个输入总线，`ChannelLayout is Writable: T`。
- `LowEndLock - Shared Code` 与 `LowEndLock - AU` 编译通过。
- 组件已安装到 `~/Library/Audio/Plug-Ins/Components/LowEndLock.component`。
- 待用户在 Logic 中验证侧链菜单与路由。

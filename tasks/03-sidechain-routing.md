# Task 03 — Sidechain 输入路由

- 状态：代码已完成，待 Logic 最终验收
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

## 根因结论（最终版，重要）

Logic Pro 识别一个 JUCE 插件的 AU sidechain，用的是 **静态声明的立体声第二输入总线**，
和 JUCE 官方 `NoiseGate` 示例一致：

```cpp
AudioProcessor (BusesProperties()
    .withInput  ("Input",     juce::AudioChannelSet::stereo(), true)
    .withOutput ("Output",    juce::AudioChannelSet::stereo(), true)
    .withInput  ("Sidechain", juce::AudioChannelSet::stereo(), true))
```

走过的弯路（都无效）：

1. 把 sidechain 改成**单声道** —— Logic 不认。
2. 改成**动态总线**（`canAddBus()`/`canRemoveBus()`）—— Logic 也不认（这是另一个机制）。

最终确认：**静态 stereo sidechain** 才是 Logic 认的方式。

另一个关键坑：**Logic 会按「插件身份」（type/subtype/manufacturer）缓存 AU 校验结果**，
日志里长期显示 `62 not scanned`。只重启 Logic、只升级版本号、只清缓存都不够，
必须**改变身份**才强制它重新识别。

## 最终修复方案

- 构造函数静态声明 `Sidechain` 输入总线，**stereo**，与主 `Input`、`Output` 并列。
- `isBusesLayoutSupported()` 采用官方 NoiseGate 写法：
  「主输入输出必须一致，sidechain 不限布局」。
- 把 AU subtype 从 `Vtqg` 改为 `Vtqh`（`.jucer` 中 `pluginCode="Vtqh"`），
  强制 Logic 把它当作全新插件重新校验。
- 版本号升到 `1.0.1`（AU version 65537）。
- `processBlock()` 直接读 bus 0（主）和 bus 1（sidechain）。
- UI 显示 `Sidechain (Kick): Connected / No Signal`，并加颜色提示。

## 附带修复

- JUCE 9.0.2 自身的 AU wrapper bug：`JuceAU::SetBusCount()` 同步了错误的 bus 布局标签，
  导致动态加 sidechain 时 `AudioUnitInitialize` 返回 -10868。
  补丁见 `patches/juce-9.0.2-logic-sidechain-buscount.patch`。
  （本插件最终采用静态 stereo sidechain，不依赖动态加总线，但补丁仍保留以备后用。）

## 进度记录

- 最终 AU 校验通过：`auval -v aufx Vtqh Manu` → `AU VALIDATION SUCCEEDED`。
- 总线形态正确：
  - Input：2 个总线（Input + Sidechain，均为 stereo）。
  - Output：1 个总线。
- 独立诊断 `tests/au_sidechain_probe.m` 确认：输入 element count = 2，两个输入总线都能设 stereo 格式并初始化成功。
- 组件已安装到 `~/Library/Audio/Plug-Ins/Components/LowEndLock.component`。
- 待用户在 Logic 中（完全退出后重开）验证：顶部标题栏右侧出现「侧链」菜单，可选 Kick。

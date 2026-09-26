# Task 04 — 低频分析与信号可视化

- 状态：In Progress
- 依赖：Task 03

## 目标

提取并显示 Bass 与 Kick 的低频能量。

## 输出

- 20–150 Hz 分析滤波器。
- 低频 RMS / 包络。
- UI 两个低频电平表。

## 验收标准

- 能区分主输入与 sidechain 的低频活动。
- 无信号时电平正确归零。

## 进度记录

- 已加入 `juce_dsp` 模块依赖。
- 已实现 20 Hz 高通 + 150 Hz 低通级联，形成 20–150 Hz 分析带。
- `processBlock` 中分别计算 Bass 与 Kick 的低频 RMS。
- UI 已显示：

```text
Bass Low: -x dB
Kick Low: -x dB
```

- 无信号时显示 `no signal`。
- `LowEndLock - Shared Code` 编译通过。
- 待 Projucer 保存并重新生成工程后，做完整 AU/VST3 构建与 DAW 验证。

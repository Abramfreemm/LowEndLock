# Task 02 — 参数系统与 UI 基础

- 状态：Not Started
- 依赖：Task 01

## 目标

学会 JUCE 参数系统，添加第一个 `gain` 参数和 Slider。

## 输出

- `gain` 参数。
- `AudioProcessorValueTreeState`。
- UI Slider。
- `processBlock()` 中应用增益。

## 验收标准

- UI 拖动 Slider 能改变音量。
- 参数可被 DAW 自动化。
- 保存/恢复工程后参数一致。

## 实现要点

- 参数 ID：`gain`。
- 范围：`-60 dB` 到 `+12 dB`。
- 音频线程做 gain 平滑，避免爆音。


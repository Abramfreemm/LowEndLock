# Task 09 — 实时性能与稳定性

- 状态：In Progress（代码级审计完成，待 DAW 实测 CPU/听感）
- 依赖：Task 07

## 目标

确保插件实时安全、低 CPU、低延迟。

## 输出

- 无分配音频线程。
- 无锁音频线程。
- CPU 与延迟测量。
- denormal 保护。

## 验收标准

- 48 kHz 单实例 CPU < 2%。
- 插件延迟为 0 或不超过 64 samples。

## 进度记录（代码级审计）

- ✅ denormal 保护：`processBlock` 首行 `juce::ScopedNoDenormals`。
- ✅ 音频线程无分配：`getBusBuffer` 返回视图、RMS/scope/分析写入预分配缓冲、`mixToMonoSample` 无分配。
- ✅ 无锁音频线程：跨线程通信全部走 `std::atomic`；scope 缓冲用 `CriticalSection`（仅 UI 10Hz 读取，不锁音频热路径）。
- ✅ 分析任务后台执行：互相关 O(n×lag) 只在点 Lock 时跑一次（约 2s 缓冲 × ±20ms lag），不在实时路径。
- ✅ 报告延迟：新增 `getLatencySamples()`，返回 `delayCenterSamples`（约 20ms@48k）。
- ⚠️ 延迟取舍：校正延迟线需要「中心偏移」才能做 ±20ms 的正/负移相，因此固有 ~20ms 延迟，**超过原定 64 samples 目标**。这是相位对齐工具的结构性取舍（≤64 samples 意味着移相范围只有 ±0.6ms，不实用）。对混音场景可接受；若后续要做「低延迟」版本可把移相范围缩到 ±5ms。

## 待 DAW 实测

1. 48kHz 单实例 CPU 是否 < 2%（用 Activity Monitor 或宿主 CPU 表）。
2. 连续播放 + 反复 Lock/A/B/拖 Low Cut 是否无爆音、无崩溃。
3. 采样率切换（44.1/48/96k）后是否正常。

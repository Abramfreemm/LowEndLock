# Task 09 — 实时性能与稳定性

- 状态：Not Started
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


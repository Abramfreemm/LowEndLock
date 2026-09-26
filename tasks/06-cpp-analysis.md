# Task 06 — C++ 相位分析引擎

- 状态：In Progress
- 依赖：Task 05

## 目标

把 Python 参考算法移植为实时安全的 C++ 分析引擎。

## 输出

- 活动检测。
- 归一化互相关。
- 极性与延迟建议。
- confidence 指标。

## 验收标准

- 与 Python 参考结果一致。
- 分析在后台线程运行。
- 弱信号时不自动应用。

## 进度记录

- 已实现 `PhaseAnalysisResult` 与归一化互相关。
- 已加入 2 秒分析缓冲。
- 已实现后台 `juce::Thread` 分析线程。
- 已提供：

```cpp
requestAnalysis()
getSuggestedDelaySamples()
getSuggestedPolarity()
getAnalysisConfidence()
```

- 弱信号时返回无效结果。
- `LowEndLock - Shared Code` 编译通过。
- 尚未接入 UI 的 `Lock` 按钮，留到 Task 08。

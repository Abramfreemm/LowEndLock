# Task 05 — Python 参考算法验证

- 状态：Done
- 依赖：Task 04

## 目标

在 Python/NumPy 中验证互相关与极性判断。

## 输出

- 测试信号生成。
- 归一化互相关。
- 最佳 lag 与 polarity 结果。

## 验收标准

- 反相信号被正确检测。
- 整数和分数延迟误差在阈值内。
- 结果可复现。

## 完成记录

- 已创建 `tests/reference_low_end_analysis.py`。
- 已验证：
  - 同相、对齐信号检测为 `best_lag = 0`，不反相。
  - 反相信号检测为 `invert_polarity = true`。
  - 延迟 120 samples 被正确检测。
  - `cancellation_saved_db` 计算正常。
- 参考脚本运行通过。

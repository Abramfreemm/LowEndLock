# Task 08 — 产品 UI 与交互闭环

- 状态：In Progress
- 依赖：Task 07

## 目标

完成产品主界面与结果反馈。

## 输出

- `Lock` 按钮。
- `Cancelation Saved` 显示。
- A/B。
- 波形 before-after 可视化。
- 手动 Polarity / Delay / Low Cut / Mix。

## 验收标准

- 新用户可在 60 秒内完成首次锁定。
- 自动结果可被手动覆盖。

## 进度记录

- ✅ `Cancelation Saved (dB)` 主指标：分析前后低频能量差，大数字显示（绿色正 / 橙色负）。
- ✅ `Lock` 按钮：一键触发 2 秒采集与分析，完成后自动应用修正；细节行显示 Polarity / Delay / Confidence。
- ✅ A/B 对比：`B (fix)` / `A (orig)` 按钮，旁路切换听对齐前后差别，不重新分析。
- ✅ 波形可视化 `WaveformScope`：实时显示 Kick（青）、Bass（橙）低频波形，锁定后叠加绿色 `Fixed` 修正后的 Bass 轨迹。
- ⬜ 手动覆盖：Polarity / Delay / Mix / Low Cut。
- ⬜ UI 打磨与预设。

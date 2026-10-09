# Task 08 — 产品 UI 与交互闭环

- 状态：Done
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
- ✅ 波形可视化 `WaveformScope`：实时显示 Kick（青）、Bass（橙）低频波形，叠加绿色 `Fixed` 修正后轨迹（Auto 锁定或 Manual 模式下都实时可见）。
- ✅ 手动覆盖：`Manual` 模式 + `Invert Polarity` + `Delay`(ms) + `Mix` + `Low Cut`(Hz)，全部接入 APVTS（可自动化/保存）。
- ✅ 拖 `Delay` / 点 `Invert` 自动切到 `Manual` 模式，修正即时生效并体现在波形上。
- ✅ 预设（中文名）：默认 / 自动紧致 / 极性翻转 / 柔和混合 / 宽低频。
- ✅ UI 打磨：Sonible 风格深色科技感 + 圆形旋钮（值弧线 + 圆点指针）+ 毛玻璃圆角面板 + 大波形区。

# Low-End Lock — 产品文案

## 一句话定位

> 一键修复 Kick 与 Bass 低频反相抵消，把被吃掉的低频能量找回来。

English one-liner:
> One-click phase alignment for kick and bass — recover the low end you've been losing to cancellation.

## 产品简介（约 80 字）

Low-End Lock 是一款实时音频插件，专治 Kick 与 Bass 在低频段互相抵消的问题。
它不压音量、不靠侧链压缩「躲开」，而是实时分析两者的相位关系，
一键翻转极性、微调延时，把低频从「反相抵消」变成「同相叠加」，
并直接显示你找回了多少 dB。

## 核心卖点

- **一键**：点 Lock，2 秒自动分析，立即生效，无需懂相位理论。
- **可见**：`Cancelation Saved (dB)` 大数字 + 实时波形，效果看得见。
- **不改性格**：只处理低频段（可调），保留 bass 的中高频质感。
- **可手动**：Invert / Delay / Mix / Low Band 全开放，自动不满意可手动覆盖。
- **低负担**：纯本地 DSP，无网络、无数据收集，轻量、实时。

## 与传统做法的区别

| | 传统侧链压缩 | Low-End Lock |
|---|---|---|
| 原理 | 压 bass 音量躲 kick | 对齐相位，让两者叠加 |
| 代价 | bass 忽大忽小（pumping） | 几乎无损 |
| 结果 | 低频避让 | 低频找回 |
| 反馈 | 只能靠听 | 有 dB 数字 + 波形 |

## 技术路线（用于技术说明）

- 对 Kick（sidechain）与 Bass（主输入）做 20–150Hz 带通，取低频段。
- 归一化互相关，估计最优极性与 fractional delay（±20ms）。
- 仅对 Bass 低频段做极性翻转 + 分数延时校正，中高频不变。
- 后台线程分析，音频线程轻量、无锁、无分配。

## 适用场景

- 电子 / 嘻哈 / 摇滚 / 流行中 Kick 与 Bass 低频打架。
- 混音时低频在耳机里还行、在手机/现场系统里发虚塌陷。
- 快速补救反相的低频、为母带前整理低频。

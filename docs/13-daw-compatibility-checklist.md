# Low-End Lock — DAW 兼容性测试清单

> 目标 DAW：Logic Pro（AU）、Ableton Live（AU/VST3）、REAPER（AU/VST3）。
> 每项通过打 ☑，失败在备注里写现象。

## 1. 安装与识别

| 检查项 | Logic | Ableton | REAPER |
|---|---|---|---|
| 插件能被扫描到 | ☐ | ☐ | ☐ |
| AU / VST3 均能加载 | ☐ | ☐ | ☐ |
| 重启宿主后仍在 | ☐ | ☐ | ☐ |

## 2. Sidechain 路由

| 检查项 | Logic | Ableton | REAPER |
|---|---|---|---|
| 能选择 sidechain 源（Kick 轨） | ☐ | ☐ | ☐ |
| `Sidechain (Kick)` 显示 Connected + 有效 dB | ☐ | ☐ | ☐ |
| 移除 sidechain 后显示 No Signal | ☐ | ☐ | ☐ |

## 3. 核心功能

| 检查项 | 结果 |
|---|---|
| 点 Lock 后 2s 出现 `Cancelation Saved` 数字 | ☐ |
| 波形里 Kick/Bass/Fixed 三轨迹可见 | ☐ |
| A/B 切换能听出差别 | ☐ |
| Manual 模式下 Invert/Delay 即时生效 | ☐ |
| Low Cut 拖动后波形/听感变化 | ☐ |
| 预设切换正常 | ☐ |

## 4. 稳定性

| 检查项 | 结果 |
|---|---|
| 连续播放 10 分钟无爆音/崩溃 | ☐ |
| 反复 Lock/A/B 无问题 | ☐ |
| 采样率 44.1/48/96k 切换正常 | ☐ |
| CPU 占用合理（< 2%） | ☐ |

## 5. 备注

（记录每个 DAW 的异常现象、复现步骤、截图/音频附件名）

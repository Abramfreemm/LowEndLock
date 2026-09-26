# Low-End Lock — 手动验收清单

## 0. 测试前准备

1. 确保 Xcode 已完成最新 AU 构建。
2. 最新 AU 已安装到：

```text
~/Library/Audio/Plug-Ins/Components/LowEndLock.component
```

3. 完全重启 Logic Pro。
4. 准备两条轨道：

- Kick 轨
- Bass 轨

## 1. Task 02：Gain 参数

| 步骤 | 预期结果 | 结果 |
|---|---|---|
| 把 Low-End Lock 插到 Bass 轨 | 插件窗口出现，显示 Gain 旋钮 | ☐ |
| 拖动 Gain 旋钮 | 音量实时变化 | ☐ |
| 让 DAW 自动化 Gain | 自动化可以写入并回放 | ☐ |
| 保存工程再重新打开 | Gain 数值被记住 | ☐ |

## 2. Task 03：Sidechain 路由

| 步骤 | 预期结果 | 结果 |
|---|---|---|
| 在插件窗口选择 Sidechain 为 Kick 轨 | 路由成功 | ☐ |
| 播放 Kick + Bass | 界面显示 `Kick Low: -x dB` | ☐ |
| 移除 Sidechain 或静音 Kick | 界面显示 `Kick Low: no signal` | ☐ |

## 3. Task 04：低频分析显示

| 步骤 | 预期结果 | 结果 |
|---|---|---|
| 只播放 Bass | `Bass Low` 显示有效 dB | ☐ |
| 只让 Kick 进入 Sidechain | `Kick Low` 显示有效 dB | ☐ |
| 静音所有输入 | 两个都显示 `no signal` | ☐ |

建议同时观察：如果输入主要是高频内容，低频显示应明显较低。

## 4. Task 05：Python 参考算法

在项目目录运行：

```bash
cd "/Users/abram/Codex Projects/lowend-lock"
python3 tests/reference_low_end_analysis.py
```

预期输出最后一行：

```text
Reference analysis checks passed.
```

| 步骤 | 预期结果 | 结果 |
|---|---|---|
| 运行 Python 参考脚本 | 所有断言通过 | ☐ |

## 5. Task 06：C++ 后台相位分析引擎

当前代码已实现后台分析线程，但尚未在 UI 中接入 `Lock` 按钮和结果显示。

因此 **Task 06 暂时不能只靠 DAW 手动观察结果**。

可验证的内容：

- 代码已通过 `LowEndLock - Shared Code` 编译。
- AU 组件已通过 `auval` 验证。
- 建议在进入 Task 08 后，通过 `Lock` 按钮触发分析，再验证结果。

| 步骤 | 预期结果 | 结果 |
|---|---|---|
| 完整构建通过 | 无编译/链接错误 | ☐ |
| AU 验证通过 | `AU VALIDATION SUCCEEDED` | ☐ |

## 6. Task 07：低频修正引擎

同样，当前修正引擎已实现，但需要分析结果触发。未接 UI 前，只能做基础验证：

- 完整构建通过。
- AU 验证通过。
- 播放音频不会出现崩溃、爆音或异常 CPU。

进入 Task 08 后，再验证：

- `Lock` 后低频是否更饱满。
- 反相场景是否自动修正。
- 分数延迟是否正确。
- 中高频是否保持原样。

| 步骤 | 预期结果 | 结果 |
|---|---|---|
| 插入插件并播放 | 无爆音、无崩溃 | ☐ |
| 保持默认 Mix=100% | 插件正常运行 | ☐ |
| 手动 Gain 调整 | 不影响插件稳定性 | ☐ |

## 7. 建议的检验顺序

先做能直接观察的 Task 02、03、04，再运行 Task 05 脚本，最后做 Task 06、07 的构建与稳定性检查。

如果你想完整验证 Task 06 和 Task 07 的算法效果，我建议下一步先实现一个最小 `Lock` 按钮和结果显示，这样测试才闭环。


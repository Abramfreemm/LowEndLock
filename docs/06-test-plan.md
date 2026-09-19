# Low-End Lock — 测试与验收计划

## 1. 目标

确保插件：

1. 客观算法正确。
2. 主观效果符合预期。
3. 实时性能稳定。
4. 在目标 DAW 中可靠运行。
5. 不会产生爆音、NaN、崩溃或异常 CPU 峰值。

## 2. 测试层级

### 2.1 单元测试

覆盖模块：

- DC Blocker。
- Biquad 滤波器。
- 分数延迟。
- 极性平滑。
- 互相关分析。
- 能量与抵消量计算。

### 2.2 算法参考测试

用 Python/NumPy 生成已知信号，验证 C++ 结果。

测试向量：

1. 完全反相：应检测 `invertPolarity = true`，修复后能量接近同相。
2. 整数样本延迟：应检测精确延迟。
3. 分数样本延迟：误差小于目标。
4. 无相关信号：`confidence` 低，不自动应用。
5. 静音输入：不产生结果，不崩溃。

### 2.3 实时安全测试

- 音频线程无 malloc：在测试构建中替换分配器，或使用 `RTAllocChecker`。
- 音频线程无锁：代码审查 + 线程检测。
- 无阻塞调用：`processBlock` 中禁止 sleep、file IO、wait。

### 2.4 性能测试

| 场景 | 目标 |
|---|---|
| 48 kHz / 128 samples / stereo | 单实例 < 2% CPU |
| 96 kHz / 256 samples / stereo | 单实例 < 3% CPU |
| 快速参数扫描 | 无爆音、无 CPU 尖峰 |

使用 DAW 内置 CPU 表、`perf` / Instruments / Visual Studio Profiler 验证。

### 2.5 集成测试

目标 DAW：

- macOS：Logic Pro、Ableton Live、REAPER、Studio One。
- Windows：Ableton Live、REAPER、FL Studio。

验证：

- 插件加载。
- 参数自动化。
- 预设保存/恢复。
- Sidechain 路由。
- Bypass。
- 工程重新打开。

### 2.6 主观测试

准备测试素材：

- 合成 Kick + 合成 Bass，故意反相。
- 采样 Kick + 电 Bass。
- 808 Kick + Sub Bass。
- 真鼓 Kick + DI Bass。

主观指标：

- 低频是否更饱满。
- 是否产生新的相位问题。
- 中高频是否被破坏。
- 自动结果是否可信。

## 3. 验收用例

| ID | 用例 | 预期 |
|---|---|---|
| TC-01 | 反相低频信号 | 检测到反相，修复后低频能量明显上升 |
| TC-02 | 时间偏移信号 | 自动补偿到最优延迟 |
| TC-03 | 静音输入 | 无输出异常，状态为 low signal |
| TC-04 | 无 sidechain | 状态提示，处理继续但分析 invalid |
| TC-05 | 快速切换 Bypass | 无爆音 |
| TC-06 | 手动参数拖动 | 平滑过渡 |
| TC-07 | 保存/恢复预设 | 参数一致 |
| TC-08 | 96 kHz 运行 | 功能正常，CPU 达标 |

## 4. 缺陷分级

| 级别 | 定义 | 示例 |
|---|---|---|
| Blocker | 崩溃、数据损坏、严重爆音 | 音频线程锁、NaN 输出 |
| Critical | 核心功能错误 | 分析结果错误 |
| Major | 明显影响使用 | UI 状态错误 |
| Minor | 体验问题 | 文案不清晰 |

## 5. 发布门槛

- 0 个 Blocker。
- 0 个 Critical。
- Major 缺陷有明确规避或修复计划。
- 目标 DAW 集成测试全部通过。
- 性能测试达标。


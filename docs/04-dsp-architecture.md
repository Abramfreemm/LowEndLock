# Low-End Lock — DSP 架构与算法设计

## 1. 目标

本插件只解决一个核心 DSP 问题：

> 在低频段估计 Kick 与 Bass 之间的相对极性和时间差，并用最小处理恢复低频能量。

## 2. 核心思想

低频相位抵消通常由两个因素造成：

1. **极性相反**：一个信号在 Kick 攻击时被另一个反相抵消。
2. **时间差**：Kick 与 Bass 的基频周期起点不一致，导致部分频率相消。

因此处理分为：

- 估计最优极性。
- 估计最优 fractional delay。
- 仅对主信号低频段应用修正。

## 3. 信号链

```text
mainL/mainR -----------------------------> [Dry/Wet] -----> outL/outR
   |                                          ^
   +--> DC Block --> Trim --> Low Band --> Correction
                                           |
sideL/sideR --> DC Block --> Trim --> Low Band --> Analysis
```

### 3.1 为什么只处理低频

- 相位抵消主要影响低频的感知能量。
- 只处理低频可避免破坏中高频立体声像和瞬态。
- 可用较轻量的滤波器完成。

## 4. 模块设计

### 4.1 DC Block

使用一阶高通：

```text
y[n] = x[n] - x[n-1] + R * y[n-1]
```

建议 `R = 0.995`。

### 4.2 分析带通 / 低通

默认分析范围：

- Low Floor：20 Hz。
- Low Cut：150 Hz。

建议使用级联 Biquad：

- 一个 2 阶 high-pass，截止 20 Hz。
- 一个 2 阶 low-pass，截止 150 Hz。

使用 RBJ Cookbook 公式计算系数。

### 4.3 活动检测

计算短期 RMS 或包络：

```text
env[n] = alpha * env[n-1] + (1 - alpha) * abs(x[n])
```

仅当主信号和 sidechain 的低频包络都超过阈值时，才把数据纳入分析。

### 4.4 互相关

给定低频主信号 `b[n]` 和 sidechain 参考 `k[n]`，在一个有限 lag 范围内计算：

```text
R[tau] = sum_n b[n] * k[n + tau]
```

归一化相关系数：

```text
r[tau] = R[tau] / sqrt(Eb * Ek)
```

搜索范围建议：

- `-20 ms` 到 `+20 ms`。
- 48 kHz 下约 `-960` 到 `+960` samples。

候选决策：

1. 在 `r[tau]` 中找绝对值最大的 `tau`。
2. 若该 `r` 为负，则极性反转后相关度更高，因此设置 `invertPolarity = true`。
3. `delaySamples = -tau`，使主信号与参考对齐。

### 4.5 分数延迟

整样本延迟直接使用环形缓冲。

分数部分使用：

- 一阶 Thiran allpass，适合 sub-sample delay。
- 或 4 阶 Lagrange 插值，适合较高精度。

MVP 推荐：

```text
integerDelay = round(delaySamples)
fraction = delaySamples - integerDelay
```

整数部分用 delay line，分数部分用 allpass。

### 4.6 极性应用

对主信号的低频分量乘以：

```text
g = invertPolarity ? -1 : 1
```

为避免爆音，`g` 通过平滑器过渡。

### 4.7 输出混合

```text
wet[n] = processed[n]
out[n] = dryGain * dry[n] + wetGain * wet[n]
```

在默认 Mix = 100% 时，输出为处理后信号。

## 5. 抵消量估计

### 5.1 修复前

将主信号与 sidechain 在低频段近似视为两个加性分量，计算：

```text
sumBefore[n] = b[n] + k[n]
```

### 5.2 修复后

```text
sumAfter[n] = bCorrected[n] + k[n]
```

### 5.3 指标

```text
Cancelation Saved = 10 * log10(sum(E_after) / max(E_before, eps))
```

单位 dB。该值表示修复后低频总能量相对修复前的变化。

注意：该指标用于 UX 表达，不等于严格声学相位抵消值，但应具有单调性和可解释性。

## 6. 分析算法伪代码

```text
function analyze(mainBuffer, sideBuffer):
    if rms(mainBuffer) < threshold or rms(sideBuffer) < threshold:
        return invalid

    bestLag = 0
    bestCorr = -inf

    for lag in -maxLag..maxLag:
        corr = normalizedCrossCorrelation(mainBuffer, sideBuffer, lag)
        if abs(corr) > abs(bestCorr):
            bestCorr = corr
            bestLag = lag

    invert = bestCorr < 0
    delaySamples = -bestLag

    beforeEnergy = sum((mainBuffer + sideBuffer)^2)
    corrected = apply(mainBuffer, invert, delaySamples)
    afterEnergy = sum((corrected + sideBuffer)^2)
    savedDb = 10 * log10(afterEnergy / max(beforeEnergy, eps))

    return AnalysisResult(valid=true,
                          invertPolarity=invert,
                          delaySamples=delaySamples,
                          cancellationBeforeDb=...,
                          cancellationAfterDb=...,
                          confidence=...)
```

## 7. 自动模式状态机

```text
IDLE -> ANALYZING -> RESULT_READY -> SMOOTH_APPLY -> MONITORING
                                      |
                                      v
                                 MANUAL_OVERRIDE
```

- `Lock` 触发 `ANALYZING`。
- 分析完成进入 `RESULT_READY`。
- 音频线程检测到新结果后，平滑切换参数。
- 用户手动修改 `polarity` 或 `delay` 时，进入 `MANUAL_OVERRIDE`。

## 8. 参数平滑

使用一阶平滑：

```text
smooth = exp(-1 / (timeConstantSeconds * sampleRate))
current = target + smooth * (current - target)
```

建议时间常数：

- 极性/增益：10–20 ms。
- 延迟：20–50 ms。
- 滤波器截止：30–80 ms。

## 9. 实时安全

- 所有滤波器状态在 `prepareToPlay` 分配。
- `processBlock` 中只使用固定大小缓冲和预先分配的延迟线。
- 分析任务在后台线程运行。
- 使用 `juce::AudioBuffer` 的 channel pointers 而不是动态分配。
- 使用 `juce::dsp::ProcessSpec` 管理采样率。

## 10. 数值保护

- 检测并抑制 denormals。
- 输入信号 clip 保护。
- 输出 clip 保护可选。
- 所有除法使用 epsilon。
- 分析能量为 0 时不产生结果。


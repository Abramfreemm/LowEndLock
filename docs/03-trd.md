# Low-End Lock — 技术需求文档

## 1. 文档信息

- 版本：v1.0
- 状态：Draft
- 依赖：PRD v1.0

## 2. 范围

本文档定义 Low-End Lock 插件在 MVP 阶段的技术约束、运行环境、信号处理架构、模块接口、线程模型、性能预算与工程要求。

## 3. 技术栈与依赖

### 3.1 建议技术栈

| 项目 | 选择 | 理由 |
|---|---|---|
| 框架 | JUCE 8.x | 成熟、跨平台、插件格式支持完整 |
| 语言 | C++20 | 适合实时 DSP |
| 构建 | CMake | JUCE 原生支持 |
| 音频格式 | VST3、AU | 覆盖主要 DAW |
| 测试框架 | Catch2 或 doctest | 轻量 |
| 基准/参考实现 | Python / NumPy | 算法验证 |

### 3.2 不引入的依赖

- 不引入大型机器学习运行时。
- 不引入云端服务。
- 不在音频线程中使用动态容器。

## 4. 插件运行环境

### 4.1 平台

- macOS 12+，Apple Silicon 原生 + Intel。
- Windows 10/11 x64。

### 4.2 插件格式

- VST3。
- AU。

### 4.3 采样率

- 必须支持：44.1 kHz、48 kHz。
- 建议支持：88.2 kHz、96 kHz。

### 4.4 通道配置

- Mono in / Mono out。
- Stereo in / Stereo out。
- Sidechain：Mono 或 Stereo 输入均可，内部取低频分析所需信号。

## 5. 处理目标与约束

| 项目 | 目标 |
|---|---|
| 实时 CPU | 单实例默认 < 2% |
| 插件延迟 | 0 samples，允许最高 64 samples |
| 内存增长 | 初始化后无动态分配 |
| 音频线程分配 | 0 |
| 锁 | 音频线程无锁 |
| 参数变化 | 平滑，无爆音 |

## 6. 信号处理管线

```text
Main Input (Bass)
    |
    v
[DC Block] -> [Trim] -> [Analysis Low Pass] -> [Correction]
                                                    |
                                                    v
Sidechain Input (Kick)                          [Output]
    |
    v
[DC Block] -> [Trim] -> [Analysis Low Pass] -> [Analysis/Decision]
                                                    |
                                                    +--> [Parameter State]
```

### 6.1 处理阶段

1. **输入预处理**
   - 去除直流。
   - 可选的输入增益。
2. **分析滤波**
   - 提取低频段，默认 20–150 Hz。
   - 分析滤波器不直接进入输出，避免额外相位污染。
3. **活动检测**
   - 检测主信号与 sidechain 是否有有效低频能量。
   - 避免在静音或极弱信号上做出错误判断。
4. **分析决策**
   - 估计 Kick 与 Bass 在当前时间窗的极性关系和时间差。
   - 生成建议的 polarity 与 fractional delay。
5. **补偿处理**
   - 仅对主信号低频段应用极性翻转。
   - 应用 fractional delay。
6. **输出混合**
   - Dry/Wet、Bypass。
   - 参数平滑与淡入淡出。

## 7. 参数模型

### 7.1 MVP 参数

| 参数 ID | 名称 | 范围 | 默认值 | 单位 | 说明 |
|---|---|---|---:|---|---|
| `enabled` | Power / Bypass | 0–1 | 1 | bool | 总开关 |
| `learn` | Lock / Learn | trigger | 0 | — | 触发一次分析 |
| `auto` | Auto | 0–1 | 1 | bool | 是否持续自动修正 |
| `polarity` | Polarity | Normal / Invert | Normal | enum | 手动极性覆盖 |
| `delay` | Delay | -20–20 | 0 | ms | 手动时间补偿 |
| `lowCut` | Low Cut | 20–300 | 150 | Hz | 处理上限频率 |
| `lowFloor` | Low Floor | 10–100 | 20 | Hz | 处理下限频率 |
| `analysisTime` | Analysis Window | 0.5–5 | 2 | s | 分析时长 |
| `dryWet` | Mix | 0–100 | 100 | % | 处理信号混合 |
| `cancelationSaved` | Cancelation Saved | read-only | 0 | dB | 结果指标 |
| `phaseReadout` | Phase Readout | read-only | -180–180 | deg | 诊断信息 |

### 7.2 参数语义

- `learn` 为触发型参数，收到事件后在非音频线程排队分析任务。
- `auto` 为 true 时，持续监控并根据新分析结果平滑更新补偿。
- `polarity` 与 `delay` 为手动覆盖；自动结果会同步更新 UI，但用户调整后切换到手动状态。

## 8. 实时线程模型

### 8.1 线程

| 线程 | 职责 |
|---|---|
| Audio Thread | 实时处理，只读预计算状态，应用平滑参数 |
| Analysis Thread | 运行互相关分析，产生新的补偿建议 |
| Message Thread | UI 参数、状态同步 |

### 8.2 数据交换

- Audio Thread 与 Analysis Thread 之间使用 atomic 参数或单生产者/单消费者无锁队列。
- 禁止在 Audio Thread 中等待 Analysis Thread。
- 分析任务产生结果后，Audio Thread 在下一个处理块平滑切换。

## 9. 模块接口

### 9.1 Processor

```cpp
class LowEndLockAudioProcessor : public juce::AudioProcessor {
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void releaseResources() override;
    juce::AudioProcessorEditor* createEditor() override;
};
```

### 9.2 DSP Engine

```cpp
class LowEndLockEngine {
public:
    void prepare(double sampleRate, int maxBlockSize);
    void process(const float* mainL,
                 const float* mainR,
                 const float* sideL,
                 const float* sideR,
                 float* outL,
                 float* outR,
                 int numSamples);
    void setParameters(const Parameters& p);
    AnalysisResult requestAnalysis();
private:
    DcBlocker dcMainL, dcMainR, dcSideL, dcSideR;
    Biquad lowpassMainL, lowpassMainR, lowpassSideL, lowpassSideR;
    FractionalDelay delayMainL, delayMainR;
    SmoothedValue gainPolarity;
    SmoothedValue delaySamples;
};
```

### 9.3 Analysis Engine

```cpp
struct AnalysisResult {
    bool valid{false};
    bool invertPolarity{false};
    float delaySamples{0.0f};
    float cancellationBeforeDb{0.0f};
    float cancellationAfterDb{0.0f};
    float confidence{0.0f};
};

class LowEndAnalyzer {
public:
    void prepare(double sampleRate);
    void addBlock(const float* main, const float* side, int numSamples);
    AnalysisResult analyze();
    void reset();
};
```

## 10. 性能预算

### 10.1 每样本成本估算

- DC Block：2 个一阶滤波器。
- 分析低通：2–4 个二阶滤波器。
- 分数延迟：1–2 个全通滤波器。
- 参数平滑：每个参数一次一阶平滑。
- 互相关分析：仅在分析线程中运行，不进入每样本路径。

### 10.2 内存预算

- 分析缓冲区：`analysisTime * sampleRate * channels` floats。
- 默认 2 秒 @ 48 kHz，mono 分析约 96,000 samples，即约 384 KB。
- 立体声路径状态内存 < 1 MB。

## 11. 采样率与延迟策略

### 11.1 延迟模式

| 模式 | 延迟 | 适用 |
|---|---:|---|
| Live | 0 samples | 默认，实时监听 |
| Lookahead | 64 samples | 后续可选，改善时间对齐精度 |

### 11.2 延迟补偿

- MVP 推荐不引入 lookahead，避免破坏实时使用体验。
- 若未来加入 lookahead，必须调用 `setLatencySamples` 正确报告。

## 12. 状态与预设

- 使用 JUCE `AudioProcessorValueTreeState`。
- 保存所有参数与内部模式。
- 不保存大型分析缓冲。
- 预设文件包含可读版本号，用于迁移。

## 13. 错误处理

- 无 sidechain 输入：UI 显示 `No Kick Sidechain`，处理继续但分析标记 invalid。
- 输入过小：分析结果 `confidence` 低，不自动应用。
- 采样率不支持：降级到最近支持频率并提示。
- 数值异常：启用 denormal 保护和输出保护。

## 14. 验收技术标准

1. 音频线程无分配、无锁、无 IO。
2. 默认 48 kHz 下 CPU 占用低于目标。
3. 对合成测试信号，分析结果与参考实现误差小于阈值。
4. 参数快速变化不产生可闻爆音。
5. 在目标 DAW 中打开、保存、恢复预设正常。


# Low-End Lock — 零基础开发路径

## 0. 当前状态

- Xcode 27.0：已安装，许可已接受。
- JUCE 9.0.1：已解压在 `/Users/abram/JUCE`。
- Projucer：`/Users/abram/JUCE/Projucer.app`。
- 项目目录：`/Users/abram/Codex Projects/lowend-lock`。

现在可以开始开发。

## 1. 第一目标：空白插件能在 DAW 中加载

不要一开始写 DSP。先完成“环境闭环”：

```text
Projucer -> Xcode -> VST3/AU -> DAW 能扫描到并加载
```

这是所有后续开发的基础。

## 2. 创建第一个空白插件

### 2.1 打开 Projucer

```bash
open /Users/abram/JUCE/Projucer.app
```

如果 Projucer 提示登录或选择许可模式：

- 学习阶段可以选择 AGPLv3 / 免费模式。
- 商业发布前再切换到商业许可。

### 2.2 新建项目

1. 选择 `Audio Plug-in` 模板。
2. 项目名：`LowEndLock`。
3. Company / Plugin Name / Bundle Identifier 填好。
4. Plugin Formats 先勾选：
   - `VST3`
   - `AU`
5. C++ Standard 选择 `C++20`。
6. 保存为：

```text
/Users/abram/Codex Projects/lowend-lock/LowEndLock.jucer
```

### 2.3 生成 Xcode 工程

在 Projucer 中点击 Xcode 图标，或使用 `Save and Open in IDE`。

Projucer 会生成：

```text
lowend-lock/
├── LowEndLock.jucer
├── Source/
├── JuceLibraryCode/
└── Builds/
    └── MacOSX/
        └── LowEndLock.xcodeproj
```

## 3. 在 Xcode 中第一次构建

1. 打开 `Builds/MacOSX/LowEndLock.xcodeproj`。
2. 选择 Scheme：
   - `LowEndLock - VST3`
   - 或 `LowEndLock - AU`
3. 点击 Build。

构建成功后，在构建目录里找到：

```text
LowEndLock.vst3
LowEndLock.component
```

## 4. 让 DAW 扫描到插件

### VST3

复制到：

```text
~/Library/Audio/Plug-Ins/VST3/
```

### AU

复制到：

```text
~/Library/Audio/Plug-Ins/Components/
```

然后重启 DAW 或重新扫描插件。

推荐先用 REAPER、Ableton Live、Logic Pro 中任意一个测试。

## 5. 先读懂三个核心文件

Projucer 会生成这些文件，重点只看三个：

### 5.1 `PluginProcessor.h`

插件的主控制器，负责：

- 参数定义。
- `prepareToPlay()`。
- `processBlock()`。
- 保存/恢复状态。

### 5.2 `PluginProcessor.cpp`

具体逻辑写在这里。Low-End Lock 的 DSP 最终会进入 `processBlock()`。

### 5.3 `PluginEditor.h` / `PluginEditor.cpp`

负责 UI：

- `resized()`：布局控件。
- `paint()`：绘制背景。
- 控件事件处理。

`JuceLibraryCode/` 是 JUCE 生成代码，不要手动改。

## 6. 建议的开发顺序

### Milestone 1：空白插件

- 空白插件成功加载。
- 能旁通、能显示窗口。

### Milestone 2：一个参数

给插件添加一个 `gain` 参数：

1. 在 `PluginProcessor` 中创建 `AudioProcessorValueTreeState`。
2. 在 `processBlock` 中乘以 gain。
3. 在 UI 中添加 Slider。

这一步是为了学会参数系统。

### Milestone 3：Sidechain 输入

1. 在 Projucer 中启用 `Sidechain` 输入。
2. 在 `processBlock` 中读取 sidechain buffer。
3. UI 显示 sidechain 是否收到信号。

Low-End Lock 依赖这一步。

### Milestone 4：低频能量分析

先做一个简单版本：

- 对主输入和 sidechain 做 20–150 Hz 带通。
- 计算低频 RMS。
- UI 显示两个电平。

这一步不涉及相位判断，只验证“能读到 Kick 和 Bass 的低频”。

### Milestone 5：Python 参考算法

在 `tests/` 中写 Python/NumPy：

- 生成反相、延迟的测试信号。
- 验证互相关能找出极性差和时间差。

先用 Python 确定算法，再翻译成 C++。

### Milestone 6：C++ 相位分析

把 Python 逻辑移植到 C++：

- 活动检测。
- 归一化互相关。
- 最佳 lag。
- 极性与延迟建议。

### Milestone 7：低频补偿

- 极性翻转。
- 分数延迟。
- 参数平滑。
- Dry/Wet。

### Milestone 8：UI 闭环

- `Lock` 按钮。
- `Cancelation Saved` 显示。
- A/B。
- 手动 Polarity / Delay / Low Cut / Mix。

## 7. 每周开发节奏

### 第 1 周

- 完成 Milestone 1–3。
- 确保插件和 sidechain 路由正常。

### 第 2 周

- 完成 Milestone 4–5。
- 用测试信号验证 Python 参考算法。

### 第 3 周

- 完成 Milestone 6–7。
- 在 DAW 中测试真实 Kick/Bass 素材。

### 第 4 周

- 完成 Milestone 8。
- 打磨 UI 与安装流程。

## 8. 遇到问题时的检查顺序

1. Projucer 是否选择正确插件格式。
2. Xcode Scheme 是否对应 VST3/AU。
3. 插件是否复制到正确目录。
4. DAW 是否重新扫描插件。
5. 是编译错误，还是运行/扫描错误。

## 9. 已记录的常见构建问题

### 9.1 Xcode 27 提示 deployment target 不支持

错误示例：

```text
The macOS deployment target 'MACOSX_DEPLOYMENT_TARGET' is set to 10.13,
but the range of supported deployment target versions is 12.0 to 27.0.x.
```

原因：JUCE 9 的默认 macOS Deployment Target 仍是 10.13，而 Xcode 27 不再支持。

修复：

1. 在 Projucer 的 Xcode 导出设置中，把 `macOS Deployment Target` 改为 `12.0`。
2. 保存并重新生成 Xcode 工程。
3. 重新构建。

### 9.2 构建阶段向系统插件目录复制失败

如果 Xcode 在构建最后报 `Operation not permitted`，先创建系统插件目录：

```bash
mkdir -p ~/Library/Audio/Plug-Ins/VST3 ~/Library/Audio/Plug-Ins/Components
```

然后重新构建。

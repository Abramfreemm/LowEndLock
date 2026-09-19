# Task 01 — 空白 JUCE 插件骨架与首次构建

- 状态：Done
- 依赖：Task 00

## 目标

让一个空白 Audio Plugin 能被 Projucer 生成、Xcode 编译，并安装为 VST3。

## 输出

- `LowEndLock.jucer`
- `Source/PluginProcessor.h`
- `Source/PluginProcessor.cpp`
- `Source/PluginEditor.h`
- `Source/PluginEditor.cpp`
- 可加载的 `LowEndLock.vst3`

## 验收标准

- Xcode 构建成功。
- VST3 已复制到：

```text
~/Library/Audio/Plug-Ins/VST3/LowEndLock.vst3
```

- 可在 DAW 中扫描并加载。

## 完成记录

- 已修复 macOS Deployment Target 为 12.0。
- 已创建 VST3 系统目录。
- 构建成功。


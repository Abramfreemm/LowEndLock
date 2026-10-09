# Low-End Lock — 参赛提交材料清单

提交前逐项核对。每项状态填 ☑ 或 ✗，备注写问题或路径。

## 1. 可运行插件（交付物）

- 安装包：release/LowEndLock-1.0.1-mac.zip（AU + VST3 + 安装说明）
- AU 版本：LowEndLock.component（Logic 用）
- VST3 版本：LowEndLock.vst3（Ableton / REAPER 用）
- 安装说明：release/README-install.txt
- 版本号：1.0.1，subtype Vtqi，auval -v aufx Vtqi Manu 通过

## 2. 说明文档

- 用户说明：docs/15-user-guide.md（参数/预设/FAQ）
- 产品文案：docs/17-product-copy.md（定位/卖点/技术）
- 产品愿景：docs/01-product-overview.md（痛点/市场/用户）

## 3. 演示材料

- Demo 视频：按 docs/16-demo-video-script.md 录制（60s）
- Before/After 音频：可选，导出「修复前/修复后」两段 wav
- 截图：插件 UI 高清截图（主界面 + 波形 + 大数字）

## 4. 技术说明（评委可能问的）

- 核心算法：20–150Hz 带通 + 归一化互相关 + 极性/分数延时
- 与传统侧链压缩的区别（对齐 vs 压音量）
- 实时性：后台分析线程、音频线程无锁无分配
- 已知取舍：约 20ms 延迟（±20ms 移相所需）
- 无 AI/模型，纯 DSP（可解释、零训练）

## 5. 合规 / 法律

- JUCE 许可：当前 AGPL/免费模式，商业/闭源发布前需切换商业许可
- 无第三方 SDK / 无网络 / 无文件读写（已移除 resourceUsage 声明）
- 源码版权归属（提交前确认是否要求开源）

## 6. 提交前最后核对

- zip 是最新 Release 版（含最新 UI + 全部修复）
- 在至少 1 个 DAW 里完整跑一遍（按 docs/13）
- 打包文件里不包含：源码、.git、Builds 缓存、调试截图
- 文件名/插件名/版本号统一（Low-End Lock 1.0.1）
- 按比赛要求上传/提交（格式、大小、命名）

## 备注

（记录比赛名称、截止时间、提交平台、特殊要求）

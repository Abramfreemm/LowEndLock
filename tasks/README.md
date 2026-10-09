# Low-End Lock 任务总览

| 编号 | 任务 | 状态 |
|---|---|---|
| 00 | 仓库与开发工作流初始化 | Done |
| 01 | 空白 JUCE 插件骨架与首次构建 | Done |
| 02 | 参数系统与 UI 基础 | In Progress |
| 03 | Sidechain 输入路由 | Done（代码完成，待 Logic 验收） |
| 04 | 低频分析与信号可视化 | In Progress |
| 05 | Python 参考算法验证 | Done |
| 06 | C++ 相位分析引擎 | In Progress |
| 07 | 极性与分数延迟补偿 | In Progress |
| 08 | 产品 UI 与交互闭环 | Done |
| 09 | 实时性能与稳定性 | In Progress |
| 10 | DAW 集成测试与 Beta | In Progress |
| 11 | 发布、许可与商业化准备 | In Progress |

## 阅读顺序

1. 先读 `tasks/00-repo-and-workflow.md`。
2. 再读 `tasks/01-blank-plugin.md`。
3. 后续按编号推进。

## 更新记录

- 2026-09-29：解决 Logic sidechain 不显示问题（静态 stereo sidechain + 改 subtype `Vtqg`→`Vtqh` 绕过缓存）；分析改为 sum-to-mono；sidechain 状态在 UI 可视化。详见 `tasks/03-sidechain-routing.md` 与 `handoffs/2026-09-29.md`。

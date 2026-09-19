# Low-End Lock

Low-End Lock 是一个专注于解决 **Kick / Bass 低频相位抵消** 的实时音频插件。

产品一句话定位：

> 把 Kick 和 Bass 锁到一起，别让低频被相位吃掉。

## 项目目标

- 用一个极小的产品，解决制作人广泛共识的一个问题：Kick 与 Bass 同时出现时低频相互抵消。
- 用极简交互和可量化的结果，和 InPhase、Auto-Align、FUSER 等全功能工具形成差异化。
- 先做可验证的 MVP，再逐步扩展为小型产品线。

## 当前阶段

Pre-development / Product Definition。

本项目尚未进入编码阶段。当前重点是先冻结产品与技术要求，避免边写边改。

## 文档索引

所有开发前文档位于 `docs/`。

| 文档 | 内容 | 阅读顺序 |
|---|---|---|
| [01-product-overview.md](docs/01-product-overview.md) | 产品愿景、市场判断、竞争分析、定位 | 1 |
| [02-prd.md](docs/02-prd.md) | 产品需求文档 | 2 |
| [03-trd.md](docs/03-trd.md) | 技术需求文档 | 3 |
| [04-dsp-architecture.md](docs/04-dsp-architecture.md) | DSP 架构与算法设计 | 4 |
| [05-ux-ui-spec.md](docs/05-ux-ui-spec.md) | UX/UI 规格 | 5 |
| [06-test-plan.md](docs/06-test-plan.md) | 测试与验收计划 | 6 |
| [07-risk-and-decision-log.md](docs/07-risk-and-decision-log.md) | 风险、假设与决策日志 | 7 |
| [08-release-and-roadmap.md](docs/08-release-and-roadmap.md) | 版本路线、发布与商业化 | 8 |
| [09-project-plan.md](docs/09-project-plan.md) | 里程碑、任务拆分与估算 | 9 |
| [10-product-logic-map.md](docs/10-product-logic-map.md) | 产品逻辑导图 | 10 |
| [11-development-guide.md](docs/11-development-guide.md) | 零基础开发路径 | 11 |

## 任务与接续

- 任务总览：[tasks/README.md](tasks/README.md)
- 开发协作流程：[WORKFLOW.md](WORKFLOW.md)
- Handoff 模板：[HANDOFF_TEMPLATE.md](HANDOFF_TEMPLATE.md)
- 最近 Handoff：[handoffs/2026-09-19.md](handoffs/2026-09-19.md)

## 关键产品决策摘要

- MVP 只做 Kick/Bass 低频相位对齐，不做全频段相位工具。
- 主交互为一个 `Lock` / `Learn` 按钮，主结果指标为 `Cancelation Saved`。
- 支持 VST3 与 AU 起步，后续再加 AAX、CLAP。
- 目标延迟尽量为 0 samples；实时 CPU 占用目标低于 1–2%。
- 不使用云端、不依赖训练模型；分析全部本地完成。

## 目录结构

```text
lowend-lock/
├── README.md
├── docs/
│   ├── 01-product-overview.md
│   ├── 02-prd.md
│   ├── 03-trd.md
│   ├── 04-dsp-architecture.md
│   ├── 05-ux-ui-spec.md
│   ├── 06-test-plan.md
│   ├── 07-risk-and-decision-log.md
│   ├── 08-release-and-roadmap.md
│   ├── 09-project-plan.md
│   ├── 10-product-logic-map.md
│   └── 11-development-guide.md
├── tasks/             # 每个小任务一个文档
├── handoffs/          # 中断接续文档
├── Source/            # JUCE 源码：Processor / Editor / DSP
├── resources/         # 图标、图片、字体
├── Builds/            # Projucer 生成的 Xcode / CMake 工程
├── tests/             # 单元测试与参考算法
└── third_party/       # 需要本地 vendoring 的第三方代码；MVP 先不放入 JUCE
```

## 工作原则

1. 先冻结需求和架构，再写代码。
2. MVP 不做“全家桶”，任何需求必须能回答“它是否直接解决低频抵消”。
3. 音频线程不做分配、不加锁、不做文件 IO。
4. 所有算法都要有可测量的客观验收标准。
5. 每个功能默认有 A/B 和可视化反馈。

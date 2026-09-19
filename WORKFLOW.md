# Low-End Lock 开发协作工作流

## 1. 目的

这个项目会分多次进行，可能被上下文压缩、临时中断或跨天继续。为了让每次回来都能快速恢复，我们采用：

1. 任务文档：`tasks/`。
2. Handoff 文档：`handoffs/`。
3. Git 提交：每次任务状态变化都提交一次。

## 2. 核心目录

```text
lowend-lock/
├── tasks/                  # 每个小任务一个文档
├── handoffs/               # 每次中断时的接续文档
├── docs/                   # PRD、TRD、DSP 等长期文档
├── Source/                 # 插件源码
├── resources/              # UI 资源
├── tests/                  # 测试与参考算法
└── WORKFLOW.md             # 本工作流
```

## 3. 任务文档规范

每个 `tasks/*.md` 包含：

- 状态：Not Started / In Progress / Done / Blocked。
- 目标。
- 输入。
- 输出。
- 验收标准。
- 实现要点。
- 依赖任务。
- 完成时提交说明。

开始一个任务时，把状态改为 `In Progress`，提交 Git。
完成一个任务时，把状态改为 `Done`，记录实际结果，提交 Git。

## 4. Handoff 文档规范

当你需要临时离开时，我会生成：

```text
handoffs/YYYY-MM-DD.md
```

Handoff 文档至少包含：

1. 当前已完成什么。
2. 当前正在做什么。
3. 下一步要做什么。
4. 最近一次 Git 提交。
5. 已知问题或待确认事项。
6. 下次恢复时先看什么。

## 5. Git 工作流

### 5.1 开始任务

```bash
git status
git pull --rebase
```

### 5.2 阶段性提交

```bash
git add -A
git commit -m "task: add gain parameter and slider"
```

### 5.3 中断前提交

```bash
git add -A
git commit -m "handoff: save progress 2026-09-19"
```

### 5.4 推送到 GitHub

```bash
git push origin main
```

## 6. 下次恢复时的动作

1. 读最新 `handoffs/`。
2. 读 `tasks/README.md` 看任务状态。
3. 运行 `git status`。
4. 确认 Xcode / Projucer / JUCE 路径。
5. 从当前 In Progress 任务继续。


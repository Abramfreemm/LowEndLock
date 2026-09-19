# Low-End Lock — 产品逻辑导图

> 本图用于从产品、技术、验证、商业化四个层面快速检查 Low-End Lock 的逻辑闭环。

```mermaid
flowchart TD
    subgraph Problem["1. 问题与用户"]
        A1["Kick 与 Bass 低频相位抵消"]
        A2["低频变薄、底鼓无力<br/>手机/现场更明显"]
        A3["目标用户<br/>制作人 / Beatmaker / 混音师"]
        A1 --> A2
        A1 --> A3
    end

    subgraph Position["2. 产品定位"]
        B1["Low-End Lock<br/>低频一键锁定"]
        B2["快速 · 精准 · 可感知"]
        A2 --> B1
        A3 --> B1
        B1 --> B2
    end

    subgraph Workflow["3. 核心使用流程"]
        C1["Bass 主信号<br/>Kick Sidechain"]
        C2["一键 Lock / Learn"]
        C3["自动极性判断"]
        C4["自动时间对齐"]
        C5["低频补偿处理"]
        C6["A/B 试听与可视化"]
        B2 --> C1
        C1 --> C2
        C2 --> C3
        C2 --> C4
        C3 --> C5
        C4 --> C5
        C5 --> C6
    end

    subgraph DSP["4. DSP 实现"]
        D1["DC Block"]
        D2["20–150 Hz 分析滤波"]
        D3["活动检测"]
        D4["互相关分析"]
        D5["极性翻转"]
        D6["分数延迟"]
        D7["参数平滑<br/>实时安全"]
        C1 --> D1
        D1 --> D2
        D2 --> D3
        D3 --> D4
        D4 --> C3
        D4 --> C4
        C5 --> D5
        C4 --> D6
        D5 --> D7
        D6 --> D7
        D7 --> C6
    end

    subgraph Metrics["5. 结果与指标"]
        E1["核心指标<br/>Cancelation Saved dB"]
        E2["辅助指标<br/>Phase / Confidence"]
        C6 --> E1
        D4 --> E2
    end

    subgraph Validation["6. 验证与发布门槛"]
        F1["客观测试<br/>反相 / 延迟 / 静音 / DAW"]
        F2["主观测试<br/>低频更饱满、无副作用"]
        F3["性能测试<br/>CPU < 2% / 0 采样延迟"]
        F4["发布门槛<br/>0 Blocker / 0 Critical"]
        E1 --> F1
        E2 --> F2
        C6 --> F3
        F1 --> F4
        F2 --> F4
        F3 --> F4
    end

    subgraph Growth["7. 商业化与扩展"]
        G1["MVP：VST3 / AU<br/>一键锁定"]
        G2["Beta：30+ 用户验证"]
        G3["V1.0：官网、教程、买断定价"]
        G4["后续：CLAP / AAX<br/>多实例联动 / 自动分离"]
        F4 --> G1
        G1 --> G2
        G2 --> G3
        G3 --> G4
    end

    subgraph NonGoals["8. 明确不做"]
        H1["不做全频段相位对齐"]
        H2["不做 AI / 云端"]
        H3["不做全功能母带链"]
        B1 -.-> H1
        B1 -.-> H2
        B1 -.-> H3
    end
```

## 阅读要点

1. **问题 → 定位**：低频相位抵消是明确痛点，产品因此必须保持“低频专用、一键修复”的定位。
2. **流程 → DSP**：用户只看到一个 Lock 按钮，但背后对应互相关、极性与分数延迟三条核心 DSP 路径。
3. **指标 → 验证**：`Cancelation Saved` 不是装饰，而是连接主观效果和客观测试的关键指标。
4. **验证 → 商业化**：MVP 先证明可用，Beta 再验证用户理解，V1.0 才进入正式售卖。
5. **非目标**：所有非目标都用于防止产品退化成“又一个全功能相位工具”。


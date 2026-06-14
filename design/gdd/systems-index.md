# Systems Index: 格斗萌主

> **Status**: Draft
> **Created**: 2026-06-15
> **Last Updated**: 2026-06-15
> **Source Concept**: design/gdd/game-concept.md

---

## Overview

格斗萌主是一款Q版2D横版格斗闯关游戏，核心循环是"城镇整备→副本闯关→结算奖励→成长搭配"。三大支柱——爽快打击+Q版反差、职业深度与成长、轻社交竞争——决定了系统优先级。战斗手感系统是命根子，必须最先设计验证；职业/装备成长系统驱动长期留存；轻量PVP提供社交调味。总共拆解为18个系统，其中9个为MVP必需。

---

## Systems Enumeration

| # | System Name | Category | Priority | Status | Design Doc | Depends On |
|---|-------------|----------|----------|--------|------------|------------|
| 1 | Input System | Core | MVP | Not Started | — | — |
| 2 | Player Controller | Core | MVP | Not Started | — | Input System |
| 3 | Combat System | Gameplay | MVP | Not Started | — | Player Controller, Hit Detection |
| 4 | Hit Detection & Feedback | Core | MVP | Not Started | — | Player Controller |
| 5 | Combo System | Gameplay | MVP | Not Started | — | Combat System |
| 6 | Enemy AI | Gameplay | MVP | Not Started | — | Combat System, Hit Detection |
| 7 | Dungeon Room System | Gameplay | MVP | Not Started | — | Enemy AI |
| 8 | Dungeon Flow System | Gameplay | MVP | Not Started | — | Dungeon Room System |
| 9 | HUD System | UI | MVP | Not Started | — | Combo System, Combat System |
| 10 | Character Stats | Progression | Vertical Slice | Not Started | — | — |
| 11 | Skill Tree System | Progression | Vertical Slice | Not Started | — | Character Stats |
| 12 | Equipment System | Economy | Vertical Slice | Not Started | — | Character Stats |
| 13 | Loot & Drop System | Economy | Vertical Slice | Not Started | — | Equipment System, Enemy AI |
| 14 | Town Hub | Gameplay | Vertical Slice | Not Started | — | Dungeon Flow System |
| 15 | Save System | Persistence | Alpha | Not Started | — | Character Stats, Equipment System |
| 16 | Audio System | Audio | Alpha | Not Started | — | Combat System, Hit Detection |
| 17 | PVP Arena | Gameplay | Full Vision | Not Started | — | Combat System, Character Stats |
| 18 | Leaderboard & Social | Meta | Full Vision | Not Started | — | PVP Arena |

---

## Categories

| Category | Description | Systems |
|----------|-------------|---------|
| **Core** | 基础系统，所有其他系统依赖 | Input System, Player Controller, Hit Detection & Feedback |
| **Gameplay** | 让游戏好玩的核心系统 | Combat System, Combo System, Enemy AI, Dungeon Room, Dungeon Flow, Town Hub, PVP Arena |
| **Progression** | 玩家长期成长 | Character Stats, Skill Tree |
| **Economy** | 资源产出与消耗 | Equipment, Loot & Drop |
| **Persistence** | 存档与设置 | Save System |
| **UI** | 玩家信息显示 | HUD System |
| **Audio** | 声音系统 | Audio System |
| **Meta** | 核心循环外 | Leaderboard & Social |

---

## Priority Tiers

| Tier | Definition | Target Milestone | Design Urgency |
|------|------------|------------------|----------------|
| **MVP** | 核心循环必需，验证"战斗是否爽" | 第一个可玩原型 | 设计 FIRST |
| **Vertical Slice** | 一个完整体验切片，展示全流程 | 垂直切片/Demo | 设计 SECOND |
| **Alpha** | 所有功能粗版上线 | Alpha里程碑 | 设计 THIRD |
| **Full Vision** | 打磨、边界、锦上添花 | Beta/Release | 按需设计 |

---

## Dependency Map

### Foundation Layer (no dependencies)
1. **Input System** — 所有交互的起点，键盘/手柄输入映射
2. **Character Stats** — 独立数值模型，不依赖其他系统

### Core Layer (depends on foundation)
1. **Player Controller** — depends on: Input System
2. **Hit Detection & Feedback** — depends on: Player Controller

### Feature Layer (depends on core)
1. **Combat System** — depends on: Player Controller, Hit Detection
2. **Combo System** — depends on: Combat System
3. **Enemy AI** — depends on: Combat System, Hit Detection
4. **Skill Tree System** — depends on: Character Stats
5. **Equipment System** — depends on: Character Stats

### Presentation Layer (depends on features)
1. **Dungeon Room System** — depends on: Enemy AI
2. **Dungeon Flow System** — depends on: Dungeon Room System
3. **HUD System** — depends on: Combo System, Combat System
4. **Loot & Drop System** — depends on: Equipment System, Enemy AI
5. **Audio System** — depends on: Combat System, Hit Detection

### Polish Layer (depends on everything)
1. **Town Hub** — depends on: Dungeon Flow System
2. **Save System** — depends on: Character Stats, Equipment System
3. **PVP Arena** — depends on: Combat System, Character Stats
4. **Leaderboard & Social** — depends on: PVP Arena

---

## Recommended Design Order

| Order | System | Priority | Layer | Agent(s) | Est. Effort |
|-------|--------|----------|-------|----------|-------------|
| 1 | Input System | MVP | Foundation | game-designer | S |
| 2 | Player Controller | MVP | Core | gameplay-programmer | M |
| 3 | Hit Detection & Feedback | MVP | Core | gameplay-programmer | M |
| 4 | Combat System | MVP | Feature | game-designer + gameplay-programmer | L |
| 5 | Combo System | MVP | Feature | game-designer | M |
| 6 | Enemy AI | MVP | Feature | ai-programmer | M |
| 7 | Dungeon Room System | MVP | Presentation | game-designer | M |
| 8 | Dungeon Flow System | MVP | Presentation | game-designer | S |
| 9 | HUD System | MVP | Presentation | ui-programmer | S |
| 10 | Character Stats | VS | Foundation | systems-designer | S |
| 11 | Skill Tree System | VS | Feature | systems-designer | M |
| 12 | Equipment System | VS | Feature | systems-designer | M |
| 13 | Loot & Drop System | VS | Presentation | systems-designer | S |
| 14 | Town Hub | VS | Polish | game-designer | M |
| 15 | Audio System | Alpha | Presentation | sound-designer | S |
| 16 | Save System | Alpha | Polish | engine-programmer | S |
| 17 | PVP Arena | Full Vision | Polish | network-programmer | L |
| 18 | Leaderboard & Social | Full Vision | Polish | network-programmer | S |

---

## Circular Dependencies

- **Combat System ↔ Enemy AI**: 战斗系统定义伤害模型，敌人AI依赖伤害模型做决策；但敌人受击行为也是战斗系统的一部分。
  - **解决**: 先设计Combat System的通用伤害接口，Enemy AI基于接口设计。受击表现由Hit Detection统一处理，不归AI管。

---

## High-Risk Systems

| System | Risk Type | Risk Description | Mitigation |
|--------|-----------|-----------------|------------|
| Combat System | Design | 连招手感调优极其耗时，帧数据/取消窗口/输入缓冲都需要大量迭代 | 原型先行验证，combat prototype已启动 |
| Combo System | Design | 简化搓招可能深度不足，硬核玩家觉得太简单 | 设计高级取消机制（如跳跃取消/闪避取消）作为可选深度 |
| Enemy AI | Technical | Boss机制复杂度容易膨胀，每个Boss都是新的行为树 | 通用AI框架+数据驱动的行为配置，Boss特殊行为用配置表 |
| PVP Arena | Technical | 格斗游戏网络同步是已知难题，帧同步vs状态同步需早期验证 | 推迟到Full Vision，MVP不涉及PVP |

---

## Progress Tracker

| Metric | Count |
|--------|-------|
| Total systems identified | 18 |
| Design docs started | 0 |
| Design docs reviewed | 0 |
| Design docs approved | 0 |
| MVP systems designed | 0/9 |
| Vertical Slice systems designed | 0/5 |

---

## Next Steps

- [ ] Review and approve this systems enumeration
- [ ] Design MVP-tier systems first (use `/design-system [system-name]`)
- [ ] Recommended first: `/design-system input-system`
- [ ] Run `/design-review` on each completed GDD
- [ ] Run `/gate-check` when MVP systems are designed

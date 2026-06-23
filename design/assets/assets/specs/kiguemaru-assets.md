# Asset Specs — Character: 小鬼丸 (Oni Crushkin)

> **Source**: design/narrative/characters/character-oni-crushkin.md
> **Art Bible**: design/art/art-bible.md
> **Generated**: 2026-06-16
> **Status**: 12 assets specced

---

## ASSET-025 — 小鬼丸 Sprite Sheet: Idle

| Field | Value |
|-------|-------|
| Category | Sprite / 2D Art |
| Dimensions | 256×256px, 8 frames |
| Format | PNG |
| Naming | char_kiguemaru_idle_001~008 |

**Visual Description:**
Q版三头身小妖怪待机，浅紫色圆脸，朱红弯角，朱红肚兜+云纹，不对称兽面肩甲，粗绳腰带，手持巨型狼牙棒扛在肩上。8帧微动：身体左右晃动（锤柄架肩）、角闪微光、铃铛轻响、好奇看锤子、蹦蹦跳。

**Generation Prompt:**
```
chibi 3-head-tall Chinese demon oni child, idle standing, light purple round face, vermilion curved horns, red dudou belly wrap with cloud patterns, asymmetrical beast-face shoulder armor, rough rope belt, giant wolf-tooth mace resting on shoulder, small bell around neck,憨厚 curious expression, 2D cel-shaded, clean outline, transparent background, sprite frame
```

---

## ASSET-026 — 小鬼丸 Sprite Sheet: Walk

| Field | Value |
|-------|-------|
| Dimensions | 256×256px, 8 frames |
| Naming | char_kiguemaru_walk_001~008 |

**Visual Description:**
8帧沉重步伐，每步地面微震，锤柄拖地冒火花。

---

## ASSET-027 — 小鬼丸 Sprite Sheet: Run

| Field | Value |
|-------|-------|
| Dimensions | 256×256px, 6 frames |
| Naming | char_kiguemaru_run_001~006 |

---

## ASSET-028 — 小鬼丸 Sprite Sheet: Light Attack (3段)

| Field | Value |
|-------|-------|
| Dimensions | 256×256px, 6+5+7 = 18 frames |
| Naming | char_kiguemaru_light1~3_*.png |

**Visual Description:**
- 轻攻1: 单手横扫锤，6帧，暗紫弧形冲击
- 轻攻2: 锤柄突刺，5帧
- 轻攻3: 双手过顶砸地，7帧，地面裂缝+震荡波AOE

---

## ASSET-029 — 小鬼丸 Sprite Sheet: Heavy Attack

| Field | Value |
|-------|-------|
| Dimensions | 256×256px, 10 frames |
| Naming | char_kiguemaru_heavy_001~010 |

**Visual Description:**
10帧跳砸：跳起4帧（蓄力压缩）→ 砸地3帧（极端拉伸1.5x）→ 大范围圆形冲击波3帧。

---

## ASSET-030 — 小鬼丸 Sprite Sheet: Special (必杀)

| Field | Value |
|-------|-------|
| Dimensions | 256×256px, 20 frames |
| Naming | char_kiguemaru_special_001~020 |

**Visual Description:**
20帧鬼王降临：鬼化全身（身体放大15%+角全亮+血红眼+暗紫气焰）→ 跳起到屏幕外 → 巨锤连砸7次（每次裂缝扩大）→ 最终一击全暗→ 鬼族虚影浮现 → 闪白。

---

## ASSET-031 — 小鬼丸 Sprite Sheet: Dodge

| Field | Value |
|-------|-------|
| Dimensions | 256×256px, 5 frames |
| Naming | char_kiguemaru_dodge_001~005 |

**Visual Description:**
5帧笨重翻滚，紫色残影+地面刮痕。

---

## ASSET-032 — 小鬼丸 Sprite Sheet: Hit

| Field | Value |
|-------|-------|
| Dimensions | 256×256px, 3 frames |
| Naming | char_kiguemaru_hit_001~003 |

**Visual Description:**
3帧快速弹回（恢复快，鬼族耐打）。

---

## ASSET-033 — 小鬼丸 Sprite Sheet: Jump

| Field | Value |
|-------|-------|
| Dimensions | 256×256px, 4 frames |
| Naming | char_kiguemaru_jump_001~004 |

**Visual Description:**
4帧沉重跳跃，着地灰尘。

---

## ASSET-034 — 小鬼丸 Portrait / Avatar

| Field | Value |
|-------|-------|
| Dimensions | 96×96px |
| Naming | ui_avatar_kiguemaru_96.png |

---

## ASSET-035 — 小鬼丸 Skill Icon: 跳砸

| Field | Value |
|-------|-------|
| Dimensions | 64×64px |
| Naming | ui_skill_kiguemaru_slam_64.png |

---

## ASSET-036 — 小鬼丸 Skill Icon: 鬼王降临

| Field | Value |
|-------|-------|
| Dimensions | 64×64px |
| Naming | ui_skill_kiguemaru_king_64.png |

**Visual Description:**
鬼化鬼丸+暗紫气焰图标。

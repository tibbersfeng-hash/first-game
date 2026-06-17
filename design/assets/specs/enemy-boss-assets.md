# Asset Specs — Enemies & Bosses

> **Source**: design/gdd/enemy-ai.md + design/gdd/game-concept.md
> **Generated**: 2026-06-17
> **Status**: 14 assets specced (ASSET-037~050)

---

## 普通敌人 (Minions)

### ASSET-037 — 糖果僵尸 (Candy Zombie) — Idle/Walk/Attack/Hit/Death

| Field | Value |
|-------|-------|
| Category | Enemy Sprite / 2D Art |
| Role | 第一关普通敌人，慢速近战 |
| Style | Q版 cel-shaded, same as player characters |

**Visual Description:**
Q版三头身糖果僵尸，绿色半透明身体，身上插着彩色棒棒糖，一只眼睛脱落用线吊着，嘴巴歪斜露出糖果牙齿，穿着破旧的粉色围裙。动作迟缓，手臂前伸。

**Stats:** HP=30, Speed=150, Damage=8, DecisionInterval=1.5s

**Generation Prompt:**
```
chibi 3-head-tall candy zombie enemy, green translucent body with colorful lollipops stuck in it, one eye hanging by a string, crooked candy teeth, tattered pink apron, slow shambling pose, 2D cel-shaded game sprite, clean black outline, white background, Q-version cute but creepy
```

**Frames needed:** idle(6), walk(6), attack(5), hit(3), death(4) = 24 frames

---

### ASSET-038 — 暴躁姜饼人 (Angry Gingerbread) — Idle/Walk/Attack/Hit/Death

| Field | Value |
|-------|-------|
| Category | Enemy Sprite |
| Role | 第一关远程敌人，投掷糖果 |
| Style | Q版 cel-shaded |

**Visual Description:**
Q版姜饼人，棕色饼干身体，糖霜装饰的纽扣和手臂，愤怒表情（糖霜眉毛倒竖），双手各拿一颗彩色糖果弹。头顶有一颗星星糖装饰。

**Stats:** HP=40, Speed=180, Damage=10, DecisionInterval=1.2s, AttackRange=300

**Generation Prompt:**
```
chibi 3-head-tall angry gingerbread man enemy, brown cookie body with white icing decorations, angry icing eyebrows, holding colorful candy projectiles, star candy on head, 2D cel-shaded game sprite, clean black outline, white background, Q-version
```

**Frames needed:** idle(6), walk(6), throw_attack(6), hit(3), death(4) = 25 frames

---

### ASSET-039 — 暗影忍者糖 (Shadow Ninja Candy) — Idle/Walk/Attack/Dodge/Hit/Death

| Field | Value |
|-------|-------|
| Category | Enemy Sprite |
| Role | 第二关敏捷敌人，高速闪避 |
| Style | Q版 cel-shaded, dark color scheme |

**Visual Description:**
Q版糖果忍者，紫色半透明身体，黑色面罩只露出红色发光眼睛，手持两把糖果苦无（shuriken），身上缠绕着紫色糖丝披风。姿态低伏，随时准备冲刺。

**Stats:** HP=50, Speed=350, Damage=12, DecisionInterval=0.8s, DodgeChance=0.3

**Generation Prompt:**
```
chibi 3-head-tall shadow candy ninja enemy, purple translucent body, black face mask with glowing red eyes, dual candy shuriken weapons, purple candy-silk cape, low crouching stealth pose, 2D cel-shaded game sprite, clean black outline, white background, Q-version agile
```

**Frames needed:** idle(6), walk(6), attack(5), dodge(4), hit(3), death(4) = 28 frames

---

### ASSET-040 — 铁甲口香糖 (Armored Gum) — Idle/Walk/Attack/Hit/Death

| Field | Value |
|-------|-------|
| Role | 第二关坦克敌人，高防低速 |
| Style | Q版 cel-shaded, metallic |

**Visual Description:**
Q版口香糖骑士，银色金属盔甲包裹粉色口香糖身体，头盔有泡泡糖泡泡装饰，手持巨大糖果盾牌和糖果狼牙棒。体型比其他敌人稍大（2.5头身）。动作笨重。

**Stats:** HP=100, Speed=120, Damage=15, DecisionInterval=2.0s, Armor=0.5

**Generation Prompt:**
```
chibi 2.5-head-tall armored gum knight enemy, silver metal armor over pink bubble gum body, bubble gum bubble on helmet, giant candy shield and candy mace weapon, bulky heavy build, 2D cel-shaded game sprite, clean black outline, white background, Q-version tank
```

**Frames needed:** idle(6), walk(6), attack(8), hit(4), death(5) = 29 frames

---

## Boss 敌人

### ASSET-041~042 — 糖果女王 · 棉花糖姬 (Candy Queen · Princess Marshmallow) — Phase 1/2

| Field | Value |
|-------|-------|
| Role | 第一关 Boss |
| Size | 4头身（比普通角色大） |
| Style | Q版 cel-shaded, regal pink/white |

**Phase 1 Visual:**
棉花糖女王，4头身，巨大蓬松的粉色棉花糖头发（像皇冠），穿白色蕾丝女王裙，手持糖果权杖（顶端有巨大棒棒糖），表情傲慢微笑。周围漂浮着棉花糖小天使。

**Phase 2 Visual (HP<50%):**
暴走形态，头发变成深红色，眼睛发红，权杖裂开露出内部黑色能量，裙摆变为暗紫色，棉花糖小天使变成黑色蝙蝠。

**Stats:** HP=500, Speed=200, Phase1_Attacks=[糖果雨, 权杖挥击, 棉花糖陷阱], Phase2_Attacks=[暗糖果风暴, 权杖重砸, 蝙蝠群]

**Generation Prompt (Phase 1):**
```
chibi 4-head-tall candy queen boss, huge fluffy pink marshmallow hair like a crown, white lace queen dress, candy scepter with giant lollipop on top, arrogant smile, floating marshmallow cherubs around her, 2D cel-shaded game sprite, clean black outline, white background, boss character, majestic
```

**Generation Prompt (Phase 2):**
```
chibi 4-head-tall enraged candy queen boss, dark red marshmallow hair, glowing red eyes, cracked scepter leaking dark energy, dark purple dress, marshmallow cherubs turned into black bats, furious expression, 2D cel-shaded game sprite, clean black outline, white background, boss character, menacing
```

**Frames needed per phase:** idle(8), walk(6), attack1(8), attack2(10), attack3(12), hit(4), phase_transition(6), death(8) = 62 frames × 2 phases

---

### ASSET-043~044 — 地狱鬼王 · 辣椒魔 (Demon King · Chili Devil) — Phase 1/2

| Field | Value |
|-------|-------|
| Role | 最终 Boss |
| Size | 5头身（最大Boss） |
| Style | Q版 cel-shaded, dark red/black/fire |

**Phase 1 Visual:**
辣椒魔王，5头身，红色辣椒形状的犄角，黑色火焰披风，手持地狱糖果三叉戟，脚下踩着熔岩糖果地面。表情狰狞，黄眼睛。身上有熔岩纹路。

**Phase 2 Visual (HP<30%):**
觉醒形态，角变成巨大火焰角，披风变为纯火焰，三叉戟分裂为三把火焰武器，全身被地狱火包裹，体型更大。

**Stats:** HP=800, Speed=250, Phase1_Attacks=[火焰冲撞, 三叉戟连刺, 熔岩喷射], Phase2_Attacks=[地狱火风暴, 三叉戟天降, 全屏熔岩]

**Generation Prompt (Phase 1):**
```
chibi 5-head-tall chili demon king boss, red chili-shaped horns, black flame cape, hell candy trident weapon, standing on molten candy ground, fierce yellow eyes, lava vein patterns on body, 2D cel-shaded game sprite, clean black outline, white background, final boss, intimidating
```

**Generation Prompt (Phase 2):**
```
chibi 5-head-tall awakened chili demon king boss, giant flame horns, pure fire cape, trident split into three flame weapons, whole body engulfed in hellfire, larger size, roaring expression, 2D cel-shaded game sprite, clean black outline, white background, final boss phase 2, extremely menacing
```

**Frames needed per phase:** idle(8), walk(6), attack1(10), attack2(12), attack3(14), hit(4), phase_transition(8), death(10) = 72 frames × 2 phases

---

## Remaining Assets (ASSET-045~050)

| Asset ID | Name | Category | Description |
|----------|------|----------|-------------|
| ASSET-045 | VFX: Hit Spark | VFX | 攻击命中时的白色火花特效，4帧 |
| ASSET-046 | VFX: Qi Wave | VFX | 慧空气功波特效，6帧 |
| ASSET-047 | VFX: Candy Explosion | VFX | 糖糖糖果爆炸特效，8帧 |
| ASSET-048 | VFX: Fire Trail | VFX | 小鬼丸火焰拖尾特效，6帧 |
| ASSET-049 | Environment: Dungeon Floor | Environment | 糖果地牢地板贴图，tileable |
| ASSET-050 | Environment: Dungeon Walls | Environment | 糖果地牢墙壁贴图，tileable |

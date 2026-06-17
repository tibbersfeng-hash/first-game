#!/usr/bin/env python3
"""格斗萌主 — 角色视觉定稿 v2.0 国风版 (高分辨率 + 国风)"""

from PIL import Image, ImageDraw, ImageFont
import math
import os

# 高分辨率渲染
W, H = 2560, 1440
SCALE = 4.5  # 适中比例

# 国风配色
BG_DARK = (20, 20, 40)
BG_FLOOR = (35, 35, 55)
OUTLINE = (30, 30, 45)

CHAR_Y = 780
CHAR_1_X = 480    # 慧空
CHAR_2_X = 1280   # 糖糖
CHAR_3_X = 2080   # 小鬼丸

# ─ 字体 ─────────────────────────────────────────────────────
def get_font(size):
    paths = [
        "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
        "/usr/share/fonts/truetype/noto/NotoSansSC-Regular.otf",
    ]
    for p in paths:
        if os.path.exists(p):
            return ImageFont.truetype(p, size)
    return ImageFont.load_default()

def draw_text(draw, text, x, y, size=24, fill=(255, 255, 255)):
    font = get_font(size)
    bbox = draw.textbbox((0, 0), text, font=font)
    tw = bbox[2] - bbox[0]
    draw.text((x - tw // 2, y), text, fill=fill, font=font)

def draw_text_left(draw, text, x, y, size=20, fill=(255, 255, 255)):
    font = get_font(size)
    draw.text((x, y), text, fill=fill, font=font)

def ellipse(draw, bbox, fill, outline=None, width=2):
    if outline:
        draw.ellipse(bbox, fill=fill, outline=outline, width=width)
    else:
        draw.ellipse(bbox, fill=fill)

def rect(draw, xy, fill, outline=None, width=2):
    if outline:
        draw.rectangle(xy, fill=fill, outline=outline, width=width)
    else:
        draw.rectangle(xy, fill=fill)

def circle(draw, cx, cy, r, fill, outline=None, width=2):
    ellipse(draw, [cx-r, cy-r, cx+r, cy+r], fill, outline, width)


# ════════════════════════════════════════════════════════════════
# 角色1: 气功小武僧 · 慧空 — 国风·少林禅武
# ═══════════════════════════════════════════════════════════════
def draw_huikong(draw, cx, cy, s):
    skin = (255, 218, 185)
    skin_shadow = (230, 195, 160)
    robe = (45, 90, 180)          # 蓝武僧袍
    robe_dark = (30, 60, 130)
    robe_light = (70, 120, 210)
    belt = (180, 40, 40)
    headband = (200, 45, 45)
    bead = (160, 120, 40)
    bead_shine = (220, 180, 60)
    gold = (255, 200, 50)
    wrap = (235, 230, 220)
    guard = (80, 80, 90)
    hair_dark = (30, 25, 25)

    head_r = int(32 * s)
    body_w = int(38 * s)
    body_h = int(42 * s)
    leg_w = int(13 * s)
    leg_h = int(26 * s)
    arm_w = int(11 * s)
    arm_h = int(30 * s)

    body_top = cy - body_h

    # ─ 气功光环（多层金色圆环）──
    for i in range(6):
        r = int((60 - i * 8) * s)
        alpha = int(40 - i * 5)
        if alpha > 0:
            circle(draw, cx, body_top - int(5*s), r, None,
                   outline=(255, 200, 50, alpha), width=int(1.5*s))

    # ─ 腿 ──
    for side in [-1, 1]:
        lx = cx + int(side * 11 * s)
        # 裤子（深蓝）
        rect(draw, [lx - leg_w//2, body_top + body_h,
                     lx + leg_w//2, body_top + body_h + leg_h],
             robe_dark)
        # 白色绑腿（3道）
        for i in range(3):
            wy = body_top + body_h + leg_h - (i+1)*int(7*s)
            rect(draw, [lx - leg_w//2 - int(1*s), wy,
                         lx + leg_w//2 + int(1*s), wy + int(5*s)],
                 wrap)
        # 草鞋/赤脚
        foot_w = int(leg_w * 1.3)
        foot_h = int(7 * s)
        ellipse(draw, [lx - foot_w//2, body_top + body_h + leg_h,
                        lx + foot_w//2, body_top + body_h + leg_h + foot_h],
                skin_shadow, OUTLINE, 1)

    # ── 身体（交领武僧袍）──
    # 袍子主体
    rect(draw, [cx - body_w//2, body_top, cx + body_w//2, body_top + body_h],
         robe, OUTLINE, 2)
    # 交领（V形白衬）
    collar_w = int(10 * s)
    collar_h = int(18 * s)
    pts = [(cx - collar_w, body_top), (cx, body_top + collar_h), (cx + collar_w, body_top)]
    draw.polygon(pts, fill=wrap)
    draw.line([cx - collar_w, body_top, cx, body_top + collar_h, cx + collar_w, body_top],
              fill=OUTLINE, width=1)
    # 袍子暗纹（云纹简化）
    for i in range(2):
        cloud_y = body_top + int(body_h * 0.3) + i * int(body_h * 0.25)
        cloud_x = cx - int(8*s) + i * int(16*s)
        circle(draw, cloud_x, cloud_y, int(3*s), robe_light)
        circle(draw, cloud_x + int(4*s), cloud_y - int(1*s), int(2*s), robe_light)

    # 红腰带
    belt_y = body_top + int(body_h * 0.65)
    rect(draw, [cx - body_w//2 - int(2*s), belt_y,
                 cx + body_w//2 + int(2*s), belt_y + int(9*s)],
         belt, OUTLINE, 1)
    # 腰带结（大蝴蝶结）
    knot_y = belt_y + int(4*s)
    circle(draw, cx, knot_y, int(6*s), belt, OUTLINE, 1)
    # 结的飘带
    draw.line([cx, knot_y, cx - int(12*s), knot_y + int(15*s)], fill=belt, width=int(3*s))
    draw.line([cx, knot_y, cx + int(12*s), knot_y + int(15*s)], fill=belt, width=int(3*s))

    # ── 手臂 ──
    for side in [-1, 1]:
        ax = cx + int(side * (body_w//2 + arm_w//2 + 2*s))
        # 宽袖（灯笼袖，国风）
        sleeve_w = int(arm_w * 1.4)
        sleeve_h = int(arm_h * 0.55)
        rect(draw, [ax - sleeve_w//2, body_top + int(6*s),
                     ax + sleeve_w//2, body_top + int(6*s) + sleeve_h],
             robe, OUTLINE, 1)
        # 袖口收束
        draw.line([ax - sleeve_w//2, body_top + int(6*s) + sleeve_h,
                   ax + sleeve_w//2, body_top + int(6*s) + sleeve_h],
                  fill=robe_dark, width=int(2*s))
        # 前臂（肤色）
        forearm_y = body_top + int(6*s) + sleeve_h
        forearm_h = int(arm_h * 0.45)
        rect(draw, [ax - int(arm_w*0.7)//2, forearm_y,
                     ax + int(arm_w*0.7)//2, forearm_y + forearm_h],
             skin)
        # 护腕（深色布）
        wrist_y = forearm_y + forearm_h - int(5*s)
        rect(draw, [ax - int(arm_w*0.8)//2, wrist_y,
                     ax + int(arm_w*0.8)//2, wrist_y + int(5*s)],
             guard, OUTLINE, 1)
        # 拳头
        fist_y = forearm_y + forearm_h + int(3*s)
        circle(draw, ax, fist_y, int(7*s), skin, OUTLINE, 2)
        # 金色绑带纹路
        for angle_off in range(4):
            a = angle_off * math.pi / 2
            fx = int(ax + math.cos(a) * 5 * s)
            fy = int(fist_y + math.sin(a) * 5 * s)
            circle(draw, fx, fy, int(1.5*s), gold)

    # ── 念珠（大颗，国风木质）──
    bead_count = 5
    for i in range(bead_count):
        angle = math.pi * 0.1 + i * math.pi * 0.16
        bx = int(cx + math.cos(angle) * 24 * s)
        by = int(body_top - int(2*s) + math.sin(angle) * 10 * s)
        br = int(7 * s)
        # 珠子主体
        circle(draw, bx, by, br, bead, OUTLINE, 1)
        # 高光
        circle(draw, bx - int(2*s), by - int(2*s), int(2.5*s), bead_shine)

    # ── 头部 ──
    head_y = body_top - head_r
    # 光头
    circle(draw, cx, head_y, head_r, skin, OUTLINE, 3)
    # 头顶微阴影（光头质感）
    ellipse(draw, [cx - head_r + int(5*s), head_y - head_r + int(5*s),
                    cx + head_r - int(5*s), head_y + int(5*s)],
            skin_shadow)
    # 头皮反光
    circle(draw, cx - int(8*s), head_y - int(10*s), int(6*s), (255, 235, 210))

    # 气功纹（头顶3个金色圆点）
    for i in range(3):
        vx = cx + (i - 1) * int(9 * s)
        vy = head_y - int(head_r * 0.55)
        circle(draw, vx, vy, int(4*s), gold, (200, 160, 30), 1)

    # 红头带
    band_r = head_r + int(3*s)
    # 头带弧形
    draw.arc([cx - band_r, head_y - band_r, cx + band_r, head_y + band_r],
             200, 340, fill=headband, width=int(7*s))
    # 飘带（2条，飘动感）
    ribbon_base_x = cx + int(band_r * 0.85)
    ribbon_base_y = head_y - int(band_r * 0.3)
    # 飘带1
    pts1 = [
        (ribbon_base_x, ribbon_base_y),
        (ribbon_base_x + int(20*s), ribbon_base_y - int(5*s)),
        (ribbon_base_x + int(35*s), ribbon_base_y + int(8*s)),
        (ribbon_base_x + int(25*s), ribbon_base_y + int(15*s)),
    ]
    draw.polygon(pts1, fill=headband)
    draw.line(pts1[0] + pts1[1], fill=OUTLINE, width=1)
    # 飘带2
    pts2 = [
        (ribbon_base_x + int(5*s), ribbon_base_y + int(3*s)),
        (ribbon_base_x + int(25*s), ribbon_base_y + int(12*s)),
        (ribbon_base_x + int(40*s), ribbon_base_y + int(5*s)),
        (ribbon_base_x + int(30*s), ribbon_base_y - int(2*s)),
    ]
    draw.polygon(pts2, fill=(180, 35, 35))

    # 眼睛（国风大眼，略有丹凤眼感觉）
    for side in [-1, 1]:
        ex = cx + int(side * 11 * s)
        ey = head_y + int(3*s)
        # 眼白（略长）
        ellipse(draw, [ex - int(7*s), ey - int(6*s), ex + int(7*s), ey + int(6*s)],
                (255, 255, 255), OUTLINE, 1)
        # 瞳孔（黑色大）
        circle(draw, ex + int(2*s), ey, int(4*s), (20, 20, 30))
        # 高光
        circle(draw, ex + int(3*s), ey - int(2*s), int(1.8*s), (255, 255, 255))
        # 眉毛（粗，略微上扬—热血感）
        draw.line([ex - int(8*s), ey - int(8*s),
                   ex + int(7*s), ey - int(10*s)],
                  fill=hair_dark, width=int(3*s))

    # 嘴巴（坚毅微笑）
    draw.arc([cx - int(7*s), head_y + int(12*s) - int(5*s),
              cx + int(7*s), head_y + int(12*s) + int(5*s)],
             10, 170, fill=OUTLINE, width=int(2*s))


# ════════════════════════════════════════════════════════════════
# 角色2: 糖果魔导师 · 糖糖 — 国风·仙道丹师
# ════════════════════════════════════════════════════════════════
def draw_tangtang(draw, cx, cy, s):
    robe = (230, 110, 160)         # 桃粉道袍
    robe_dark = (190, 80, 130)
    robe_edge = (100, 220, 200)    # 青绿边（道袍风格）
    hat = (90, 50, 120)            # 深紫道冠
    hat_star = (255, 220, 80)
    apron = (250, 245, 230)        # 米白围裙（炼丹围裙）
    boot = (140, 90, 170)          # 紫灰靴
    hair = (240, 160, 180)         # 浅粉发
    hair_dark = (210, 130, 155)
    skin = (255, 232, 210)
    cheek = (255, 180, 180, 80)

    head_r = int(30 * s)
    body_w = int(36 * s)
    body_h = int(40 * s)
    leg_w = int(11 * s)
    leg_h = int(24 * s)
    arm_w = int(10 * s)
    arm_h = int(28 * s)

    body_top = cy - body_h

    # ── 法阵光环 ─
    for i in range(5):
        r = int((55 - i * 8) * s)
        alpha = int(35 - i * 5)
        if alpha > 0:
            circle(draw, cx, body_top - int(5*s), r, None,
                   outline=(230, 110, 200, alpha), width=int(1*s))

    # ── 腿+靴 ──
    for side in [-1, 1]:
        lx = cx + int(side * 10 * s)
        # 袍下摆露出的腿
        rect(draw, [lx - leg_w//2, body_top + body_h,
                     lx + leg_w//2, body_top + body_h + int(leg_h*0.4)],
             robe_dark)
        # 靴子（翘头靴，国风）
        boot_w = int(leg_w * 1.5)
        boot_h = int(leg_h * 0.7)
        boot_y = body_top + body_h + int(leg_h * 0.3)
        rect(draw, [lx - boot_w//2, boot_y, lx + boot_w//2, boot_y + boot_h],
             boot, OUTLINE, 1)
        # 靴尖上翘（国风特征）
        tip_dir = side
        draw.line([lx + int(tip_dir * boot_w//2), boot_y,
                   lx + int(tip_dir * (boot_w//2 + 6*s)), boot_y - int(4*s)],
                  fill=boot, width=int(3*s))

    # ── 身体（道袍式长袍）──
    # 下摆（A字形）
    skirt_w_top = int(body_w * 0.8)
    skirt_w_bot = int(body_w * 1.3)
    skirt_pts = [
        (cx - skirt_w_top//2, body_top + int(body_h*0.5)),
        (cx + skirt_w_top//2, body_top + int(body_h*0.5)),
        (cx + skirt_w_bot//2, body_top + body_h),
        (cx - skirt_w_bot//2, body_top + body_h),
    ]
    draw.polygon(skirt_pts, fill=robe)
    # 青绿边
    draw.line([cx - skirt_w_bot//2, body_top + body_h - int(3*s),
               cx + skirt_w_bot//2, body_top + body_h - int(3*s)],
              fill=robe_edge, width=int(4*s))
    # 上半身
    rect(draw, [cx - body_w//2, body_top, cx + body_w//2, body_top + int(body_h*0.55)],
         robe, OUTLINE, 1)
    # 交领
    collar_w = int(8*s)
    collar_h = int(14*s)
    pts = [(cx - collar_w, body_top + int(3*s)), (cx, body_top + int(3*s) + collar_h),
           (cx + collar_w, body_top + int(3*s))]
    draw.polygon(pts, fill=robe_edge)

    # 炼丹围裙（米白）
    apron_w = int(body_w * 0.65)
    apron_h = int(body_h * 0.7)
    rect(draw, [cx - apron_w//2, body_top + int(6*s),
                 cx + apron_w//2, body_top + int(6*s) + apron_h],
         apron, OUTLINE, 1)
    # 围裙上的丹纹（太极简化）
    dan_y = body_top + int(6*s) + apron_h // 2
    circle(draw, cx - int(4*s), dan_y, int(4*s), (100, 220, 200, 128))
    circle(draw, cx + int(4*s), dan_y, int(4*s), (230, 110, 160, 128))

    # ─ 手臂（宽大袖子）──
    for side in [-1, 1]:
        ax = cx + int(side * (body_w//2 + arm_w//2 + 3*s))
        # 宽袖（灯笼袖）
        sw = int(arm_w * 1.5)
        sh = int(arm_h * 0.55)
        rect(draw, [ax - sw//2, body_top + int(6*s),
                     ax + sw//2, body_top + int(6*s) + sh],
             robe, OUTLINE, 1)
        # 袖口青绿
        draw.line([ax - sw//2, body_top + int(6*s) + sh,
                   ax + sw//2, body_top + int(6*s) + sh],
                  fill=robe_edge, width=int(3*s))
        # 手（纤小）
        hand_y = body_top + int(6*s) + sh + int(5*s)
        circle(draw, ax, hand_y, int(5*s), skin, OUTLINE, 1)

    # ── 法杖（改为拂尘/拂尘杖，国风）──
    staff_x = cx + int((body_w//2 + arm_w * 1.8))
    staff_top = body_top - int(35 * s)
    staff_bot = body_top + body_h + leg_h
    # 杖柄（竹节纹）
    draw.line([staff_x, staff_top, staff_x, staff_bot], fill=(120, 80, 50), width=int(4*s))
    # 竹节
    for i in range(6):
        ny = staff_top + i * int((staff_bot - staff_top) / 6)
        draw.line([staff_x - int(3*s), ny, staff_x + int(3*s), ny],
                  fill=(90, 60, 35), width=int(1*s))
    # 杖顶：葫芦丹炉（国风法器）
    gourd_y = staff_top - int(12*s)
    # 葫芦下肚
    circle(draw, staff_x, gourd_y + int(5*s), int(14*s), (200, 60, 60), OUTLINE, 2)
    # 葫芦上肚
    circle(draw, staff_x, gourd_y - int(8*s), int(10*s), (220, 80, 80), OUTLINE, 2)
    # 葫芦盖
    rect(draw, [staff_x - int(4*s), gourd_y - int(18*s),
                 staff_x + int(4*s), gourd_y - int(14*s)],
         (180, 150, 50), OUTLINE, 1)
    # 丹气（从葫芦口飘出）
    for i in range(3):
        puff_x = staff_x + int((i - 1) * 6 * s)
        puff_y = gourd_y - int(22*s) - i * int(5*s)
        puff_r = int((4 - i) * s)
        colors = [(255, 180, 200, 100), (180, 220, 255, 80), (200, 255, 200, 60)]
        circle(draw, puff_x, puff_y, puff_r, colors[i])

    # ── 头发（双髻，国风发髻）──
    head_y = body_top - head_r
    # 发髻左
    bun_l_x = cx - int(20*s)
    bun_l_y = head_y - int(head_r * 0.7)
    circle(draw, bun_l_x, bun_l_y, int(12*s), hair, OUTLINE, 1)
    circle(draw, bun_l_x, bun_l_y, int(12*s), hair_dark)  # 暗面
    circle(draw, bun_l_x - int(3*s), bun_l_y - int(3*s), int(5*s), hair)
    # 发髻右
    bun_r_x = cx + int(20*s)
    bun_r_y = head_y - int(head_r * 0.7)
    circle(draw, bun_r_x, bun_r_y, int(12*s), hair, OUTLINE, 1)
    circle(draw, bun_r_x, bun_r_y, int(12*s), hair_dark)
    circle(draw, bun_r_x + int(3*s), bun_r_y - int(3*s), int(5*s), hair)
    # 发带
    draw.line([bun_l_x, bun_l_y + int(12*s), bun_l_x - int(8*s), bun_l_y + int(25*s)],
              fill=robe_edge, width=int(2*s))
    draw.line([bun_r_x, bun_r_y + int(12*s), bun_r_x + int(8*s), bun_r_y + int(25*s)],
              fill=robe_edge, width=int(2*s))

    # ── 头部 ─
    circle(draw, cx, head_y, head_r, skin, OUTLINE, 2)
    # 刘海
    for i in range(3):
        bx = cx + (i - 1) * int(9*s)
        by = head_y - int(head_r * 0.5)
        circle(draw, bx, by, int(8*s), hair)

    # 道冠（小冠，替代尖帽）
    crown_w = int(head_r * 1.1)
    crown_h = int(head_r * 0.6)
    crown_y = head_y - head_r - int(3*s)
    # 冠身
    rect(draw, [cx - crown_w//2, crown_y, cx + crown_w//2, crown_y + crown_h],
         hat, OUTLINE, 2)
    # 冠顶（弧形）
    draw.arc([cx - crown_w//2, crown_y - crown_h, cx + crown_w//2, crown_y + crown_h],
             180, 360, fill=hat, width=int(6*s))
    # 冠上玉簪
    draw.line([cx - int(15*s), crown_y + int(3*s),
               cx + int(15*s), crown_y + int(3*s)],
              fill=(200, 200, 220), width=int(3*s))
    circle(draw, cx + int(15*s), crown_y + int(3*s), int(3*s), hat_star)

    # ─ 眼睛（杏眼，国风）──
    for side in [-1, 1]:
        ex = cx + int(side * 10 * s)
        ey = head_y + int(2*s)
        # 杏眼（略圆）
        ellipse(draw, [ex - int(7*s), ey - int(6*s), ex + int(7*s), ey + int(6*s)],
                (255, 255, 255), OUTLINE, 1)
        # 粉色瞳（丹师特征）
        circle(draw, ex + int(1*s), ey, int(4*s), (200, 60, 120))
        # 高光
        circle(draw, ex + int(2*s), ey - int(2*s), int(1.8*s), (255, 255, 255))
        # 细眉（柳叶眉）
        draw.line([ex - int(7*s), ey - int(9*s),
                   ex + int(6*s), ey - int(8*s)],
                  fill=(40, 30, 35), width=int(2*s))

    # 嘴巴（樱桃小口）
    circle(draw, cx, head_y + int(12*s), int(3*s), (200, 80, 100))
    # 腮红
    circle(draw, cx - int(15*s), head_y + int(6*s), int(5*s), (255, 160, 160, 60))
    circle(draw, cx + int(15*s), head_y + int(6*s), int(5*s), (255, 160, 160, 60))


# ════════════════════════════════════════════════════════════════
# 角色3: 萌鬼狂战士 · 小鬼丸 — 国风·山海经妖怪
# ════════════════════════════════════════════════════════════════
def draw_kiguemaru(draw, cx, cy, s):
    skin = (170, 150, 210)
    skin_dark = (130, 110, 175)
    belly = (180, 35, 35)           # 朱红肚兜
    belly_dark = (140, 25, 25)
    armor = (70, 70, 80)            # 铁灰甲
    armor_hi = (100, 100, 110)
    guard = (40, 40, 50)
    horn = (220, 40, 50)            # 朱砂红角
    horn_glow = (255, 120, 50)
    hammer_body = (200, 50, 60)     # 朱红锤
    hammer_stripe1 = (255, 200, 80) # 金纹
    hammer_stripe2 = (80, 180, 220) # 青纹
    hammer_handle = (100, 60, 40)
    shorts = (70, 70, 80)
    bell = (255, 200, 50)
    eye_gold = (255, 210, 60)

    head_r = int(30 * s)
    body_w = int(44 * s)   # 更宽
    body_h = int(40 * s)
    leg_w = int(15 * s)
    leg_h = int(24 * s)
    arm_w = int(13 * s)
    arm_h = int(28 * s)

    body_top = cy - body_h

    # ── 腿 ──
    for side in [-1, 1]:
        lx = cx + int(side * 13 * s)
        # 短裤
        rect(draw, [lx - leg_w//2, body_top + body_h,
                     lx + leg_w//2, body_top + body_h + int(leg_h*0.45)],
             shorts, OUTLINE, 1)
        # 紫色小腿
        rect(draw, [lx - leg_w//2, body_top + body_h + int(leg_h*0.45),
                     lx + leg_w//2, body_top + body_h + leg_h],
             skin)
        # 厚脚
        foot_w = int(leg_w * 1.4)
        foot_h = int(8 * s)
        ellipse(draw, [lx - foot_w//2, body_top + body_h + leg_h,
                        lx + foot_w//2, body_top + body_h + leg_h + foot_h],
                skin_dark, OUTLINE, 2)

    # ── 身体（朱红肚兜+甲）──
    # 肚兜主体
    rect(draw, [cx - body_w//2, body_top, cx + body_w//2, body_top + body_h],
         belly, OUTLINE, 3)
    # 肚兜暗面
    rect(draw, [cx - body_w//2, body_top + body_h//2,
                 cx + body_w//2, body_top + body_h],
         belly_dark)
    # 交叉带
    draw.line([cx - body_w//2, body_top + int(5*s),
               cx + body_w//2, body_top + int(5*s)],
              fill=belly_dark, width=int(3*s))
    draw.line([cx, body_top, cx, body_top + body_h],
              fill=belly_dark, width=int(3*s))
    # "鬼"字（简化为圆形纹章）
    emblem_y = body_top + int(body_h * 0.35)
    circle(draw, cx, emblem_y, int(8*s), belly_dark, (80, 15, 15), 1)
    # 云纹装饰（肚兜上的国风纹样）
    for i in range(2):
        cx2 = cx + (i * 2 - 1) * int(12*s)
        cy2 = body_top + int(body_h * 0.7)
        circle(draw, cx2, cy2, int(3*s), (200, 60, 60, 100))
        circle(draw, cx2 + int(3*s), cy2 - int(1*s), int(2*s), (200, 60, 60, 80))

    # 粗绳腰带
    rope_y = body_top + int(body_h * 0.88)
    rect(draw, [cx - int(body_w*0.65), rope_y,
                 cx + int(body_w*0.65), rope_y + int(7*s)],
         (160, 130, 100), OUTLINE, 1)
    # 绳结
    circle(draw, cx, rope_y + int(3*s), int(4*s), (140, 110, 80))

    # ─ 肩甲（国风兽面甲，不对称）──
    # 左大肩甲（兽面纹）
    lsx = cx - body_w//2 - int(6*s)
    rect(draw, [lsx - int(14*s), body_top - int(6*s),
                 lsx + int(14*s), body_top + int(16*s)],
         armor, OUTLINE, 2)
    # 兽面（简化）
    circle(draw, lsx, body_top + int(2*s), int(5*s), armor_hi)
    circle(draw, lsx - int(2*s), body_top + int(1*s), int(1.5*s), (30, 30, 30))
    circle(draw, lsx + int(2*s), body_top + int(1*s), int(1.5*s), (30, 30, 30))
    # 右小肩甲
    rsx = cx + body_w//2 + int(4*s)
    rect(draw, [rsx - int(9*s), body_top + int(2*s),
                 rsx + int(9*s), body_top + int(14*s)],
         armor, OUTLINE, 2)

    # ─ 手臂 ──
    for side in [-1, 1]:
        ax = cx + int(side * (body_w//2 + arm_w//2 + 5*s))
        # 紫色上臂
        rect(draw, [ax - arm_w//2, body_top + int(8*s),
                     ax + arm_w//2, body_top + int(8*s) + int(arm_h*0.5)],
             skin)
        # 铁护臂（带云纹）
        guard_y = body_top + int(8*s) + int(arm_h*0.5)
        guard_h = int(arm_h * 0.5)
        rect(draw, [ax - int(arm_w*0.6), guard_y,
                     ax + int(arm_w*0.6), guard_y + guard_h],
             guard, OUTLINE, 2)
        # 拳头
        fist_y = guard_y + guard_h + int(3*s)
        circle(draw, ax, fist_y, int(8*s), skin, OUTLINE, 2)

    # ── 巨锤（国风·降魔杵造型）──
    hammer_cx_pos = cx + int((body_w//2 + arm_w + 28*s))
    hammer_top = body_top - int(40 * s)
    hammer_bot = body_top + body_h + int(10*s)
    # 柄
    draw.line([hammer_cx_pos, hammer_top, hammer_cx_pos, hammer_bot],
              fill=hammer_handle, width=int(5*s))
    # 柄上缠布
    for i in range(8):
        ty = hammer_top + int(20*s) + i * int(8*s)
        if ty < hammer_bot - int(10*s):
            draw.line([hammer_cx_pos - int(4*s), ty,
                       hammer_cx_pos + int(4*s), ty + int(4*s)],
                      fill=(180, 50, 50), width=int(2*s))
    # 锤头（金刚杵造型，菱形）
    hh_w = int(22 * s)
    hh_h = int(35 * s)
    hh_cy = hammer_top - int(15*s)
    # 菱形主体
    diamond_pts = [
        (hammer_cx_pos, hh_cy - hh_h),         # 顶
        (hammer_cx_pos + hh_w, hh_cy),          # 右
        (hammer_cx_pos, hh_cy + hh_h * 0.6),    # 底
        (hammer_cx_pos - hh_w, hh_cy),          # 左
    ]
    draw.polygon(diamond_pts, fill=hammer_body, outline=OUTLINE)
    # 金色纹路
    draw.line([hammer_cx_pos, hh_cy - hh_h + int(5*s),
               hammer_cx_pos, hh_cy + hh_h * 0.6 - int(5*s)],
              fill=hammer_stripe1, width=int(3*s))
    draw.line([hammer_cx_pos - hh_w + int(5*s), hh_cy,
               hammer_cx_pos + hh_w - int(5*s), hh_cy],
              fill=hammer_stripe2, width=int(3*s))
    # 顶端宝珠
    circle(draw, hammer_cx_pos, hh_cy - hh_h - int(5*s), int(6*s),
           hammer_stripe1, OUTLINE, 1)

    # ── 铃铛 ──
    circle(draw, cx, body_top - int(4*s), int(5*s), bell, OUTLINE, 1)

    # ── 头部 ──
    head_y = body_top - head_r
    circle(draw, cx, head_y, head_r, skin, OUTLINE, 3)

    # 鬼角（珊瑚状，国风）
    for side in [-1, 1]:
        hx = cx + int(side * 13 * s)
        horn_base = head_y - int(head_r * 0.65)
        horn_mid = head_y - int(head_r * 1.2)
        horn_tip = head_y - int(head_r * 1.5)
        # 角主干
        draw.line([hx, horn_base, hx + int(side*2*s), horn_mid],
                  fill=horn, width=int(7*s))
        draw.line([hx + int(side*2*s), horn_mid, hx + int(side*3*s), horn_tip],
                  fill=horn, width=int(5*s))
        # 角分叉（珊瑚感）
        draw.line([hx + int(side*2*s), horn_mid,
                   hx + int(side*6*s), horn_mid - int(3*s)],
                  fill=horn, width=int(3*s))
        # 角尖发光
        circle(draw, hx + int(side*3*s), horn_tip, int(4*s),
               (255, 150, 60, 100))

    # 尖耳朵
    for side in [-1, 1]:
        ex = cx + int(side * head_r * 0.9)
        ear_pts = [
            (ex, head_y - int(3*s)),
            (ex + int(side * 12*s), head_y - int(8*s)),
            (ex + int(side * 5*s), head_y + int(5*s))
        ]
        draw.polygon(ear_pts, fill=skin, outline=OUTLINE)

    # 眼睛（怒目圆睁，国风）
    for side in [-1, 1]:
        ex = cx + int(side * 11 * s)
        ey = head_y + int(1*s)
        # 大眼白
        ellipse(draw, [ex - int(8*s), ey - int(7*s), ex + int(8*s), ey + int(7*s)],
                (255, 255, 235), OUTLINE, 1)
        # 金色瞳孔
        circle(draw, ex + int(2*s), ey, int(4.5*s), eye_gold)
        circle(draw, ex + int(2*s), ey, int(4.5*s), (200, 160, 30), 1)
        # 瞳仁
        circle(draw, ex + int(3*s), ey, int(2*s), (20, 20, 20))
        # 高光
        circle(draw, ex + int(4*s), ey - int(2*s), int(1.5*s), (255, 255, 255))
        # 怒眉（粗且下压）
        draw.line([ex - int(8*s), ey - int(8*s),
                   ex + int(8*s), ey - int(10*s)],
                  fill=(30, 25, 30), width=int(4*s))

    # 锯齿嘴（凶萌）
    mouth_y = head_y + int(13*s)
    mouth_w = int(16 * s)
    for i in range(5):
        mx = cx + int((i * 4 - 8) * s)
        if i % 2 == 0:
            draw.line([mx, mouth_y, mx + int(2*s), mouth_y + int(4*s)],
                      fill=OUTLINE, width=int(2*s))
        else:
            draw.line([mx, mouth_y + int(4*s), mx + int(2*s), mouth_y],
                      fill=OUTLINE, width=int(2*s))


# ════════════════════════════════════════════════════════════════
# 主渲染
# ════════════════════════════════════════════════════════════════
def render():
    img = Image.new('RGBA', (W, H), (*BG_DARK, 255))
    draw = ImageDraw.Draw(img)

    # ── 背景（国风·暗色卷轴感）──
    # 顶部暗色渐变
    for y in range(120):
        alpha = int((120 - y) / 120 * 100)
        draw.line([(0, y), (W, y)], fill=(0, 0, 0, alpha))
    # 卷轴边纹（左右）
    for x in [20, W-20]:
        draw.line([(x, 0), (x, H)], fill=(60, 50, 40, 80), width=2)
    # 顶部横纹
    draw.line([(0, 90), (W, 90)], fill=(80, 70, 50, 60), width=1)

    # 氛围粒子（萤火/灵气）
    for i in range(30):
        px = (i * 173 + 50) % W
        py = (i * 127 + 30) % int(H * 0.75)
        sz = 1 + (i * 5) % 3
        al = 15 + (i * 11) % 30
        draw.ellipse([px - sz, py - sz, px + sz, py + sz], fill=(200, 200, 255, al))

    # ── 地面（石砖，国风地砖）──
    floor_y = CHAR_Y + 12
    # 地面渐变
    for y_off in range(H - floor_y):
        ratio = y_off / (H - floor_y)
        r = int(35 + ratio * 10)
        g = int(35 + ratio * 10)
        b = int(55 + ratio * 10)
        draw.line([(0, floor_y + y_off), (W, floor_y + y_off)], fill=(r, g, b))
    # 地砖线
    draw.line([(0, floor_y), (W, floor_y)], fill=(80, 80, 100), width=2)
    for x in range(0, W, 80):
        draw.line([(x, floor_y), (x, H)], fill=(45, 45, 65), width=1)
    for yo in [40, 80, 120, 160]:
        draw.line([(0, floor_y + yo), (W, floor_y + yo)], fill=(45, 45, 65), width=1)

    # ── 标题（国风书法感）──
    draw_text(draw, "格斗萌主 — 角色视觉定稿 v2.0", W//2, 20, size=36, fill=(240, 235, 220))
    draw_text(draw, "Q版2D横版格斗 · 三头身比例 · 糖果炸裂风 · 国风", W//2, 65, size=20, fill=(180, 175, 160))
    # 装饰线
    draw.line([W//2 - 300, 95, W//2 + 300, 95], fill=(120, 100, 70), width=1)
    # 装饰点
    circle(draw, W//2 - 300, 95, 3, (180, 150, 80))
    circle(draw, W//2 + 300, 95, 3, (180, 150, 80))

    # ── 分隔线（角色间）──
    dividers = [CHAR_1_X + 220, CHAR_2_X - 220, CHAR_2_X + 220, CHAR_3_X - 220]
    for dx in dividers:
        for dy in range(CHAR_Y - 200, CHAR_Y + 200, 8):
            alpha = int(40 * (1 - abs(dy - CHAR_Y) / 200))
            if alpha > 0:
                draw.line([(dx, dy), (dx, dy + 4)], fill=(80, 80, 100, alpha), width=1)

    # ── 角色 ─
    s = SCALE
    draw_huikong(draw, CHAR_1_X, CHAR_Y, s)
    draw_tangtang(draw, CHAR_2_X, CHAR_Y, s)
    draw_kiguemaru(draw, CHAR_3_X, CHAR_Y, s)

    # ── 标签框 ──
    labels = [
        (CHAR_1_X, "气功小武僧", "慧空 · Blazing Monkfist", "近战连招 | 气功层数", (51, 102, 204)),
        (CHAR_2_X, "糖果魔导师", "糖糖 · Sugar Hexblade", "中程控场 | 糖果配方", (230, 110, 160)),
        (CHAR_3_X, "萌鬼狂战士", "小鬼丸 · Oni Crushkin", "近战爆发 | 怒气变身", (155, 89, 200)),
    ]

    for lx, title, name, role, accent in labels:
        box_w, box_h = 280, 90
        bx = lx - box_w // 2
        by = CHAR_Y + 230
        # 背景（半透明）
        draw.rectangle([bx, by, bx + box_w, by + box_h], fill=(20, 20, 35, 200))
        # 顶色条
        draw.rectangle([bx, by, bx + box_w, by + 5], fill=accent)
        # 装饰角
        for corner_x, corner_y in [(bx+5, by+10), (bx+box_w-5, by+10)]:
            circle(draw, corner_x, corner_y, 2, accent)
        # 文字
        draw_text(draw, title, lx, by + 15, size=22, fill=(240, 235, 220))
        draw_text(draw, name, lx, by + 42, size=16, fill=accent)
        draw_text(draw, role, lx, by + 65, size=14, fill=(160, 155, 145))

    # ── 底部信息 ──
    draw_text(draw, "辨识度: 主色唯一 | 剪影唯一 | 32px可辨 | 特效色唯一 | 轮廓(圆/三角/方)",
              W//2, H - 40, size=14, fill=(130, 125, 115))

    # ── 保存 ──
    out = "/tmp/first-game-characters/characters_v2_guofeng.png"
    os.makedirs(os.path.dirname(out), exist_ok=True)
    img.convert('RGB').save(out, 'PNG', quality=95)
    print(f"✅ 已保存: {out} ({os.path.getsize(out)} bytes)")

    return out

if __name__ == '__main__':
    render()

#!/usr/bin/env python3
"""格斗萌主 — 角色视觉定稿 v3.0 国风·梦幻西游参考版
改进点（参照梦幻西游Q版设计原则）:
- 慧空: 短发+发髻(非光头)、金色护手、袈裟披肩、布鞋
- 糖糖: 双环望仙髻(非道冠)、齐胸襦裙(非围裙)、披帛、水袖、翘头绣鞋
- 小鬼丸: 粗壮弯角、獠牙、狼牙棒(非菱形锤)、小尾巴、铁指虎、兽面大肩甲
"""

from PIL import Image, ImageDraw, ImageFont
import math
import os

W, H = 2560, 1440
SCALE = 4.5

BG_DARK = (20, 20, 40)
BG_FLOOR = (35, 35, 55)
OUTLINE = (30, 30, 45)

CHAR_Y = 760
CHAR_1_X = 480
CHAR_2_X = 1280
CHAR_3_X = 2080

def get_font(size):
    paths = [
        "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
        "/usr/share/fonts/truetype/noto/NotoSansSC-Regular.otf",
    ]
    for p in paths:
        if os.path.exists(p):
            return ImageFont.truetype(p, size)
    return ImageFont.load_default()

def txt(draw, text, x, y, size=24, fill=(255, 255, 255)):
    font = get_font(size)
    bbox = draw.textbbox((0, 0), text, font=font)
    tw = bbox[2] - bbox[0]
    draw.text((x - tw // 2, y), text, fill=fill, font=font)

def circ(draw, cx, cy, r, fill, outline=None, width=2):
    draw.ellipse([cx-r, cy-r, cx+r, cy+r], fill=fill, outline=outline, width=width)

def rect(draw, xy, fill, outline=None, width=2):
    draw.rectangle(xy, fill=fill, outline=outline, width=width)


# ════════════════════════════════════════════════════════════
# 慧空 v3 — 武僧头+袈裟+护手+布鞋 (参照剑侠客+化生寺)
# ════════════════════════════════════════════════════════════
def draw_huikong_v3(draw, cx, cy, s):
    skin = (255, 218, 185)
    skin_shadow = (230, 195, 160)
    robe = (45, 90, 180)
    robe_dark = (30, 60, 130)
    robe_light = (70, 120, 210)
    belt = (180, 40, 40)
    headband = (200, 45, 45)
    gold = (220, 180, 50)
    gold_bright = (255, 215, 60)
    wrap = (235, 230, 220)
    guard = (180, 160, 50)  # 金色护手
    guard_dark = (150, 130, 30)
    hair = (30, 25, 25)
    shoe = (200, 190, 170)
    shoe_dark = (170, 160, 140)
    cassock = (220, 180, 50, 60)  # 金色袈裟(半透明)

    head_r = int(32 * s)
    body_w = int(38 * s)
    body_h = int(42 * s)
    leg_w = int(13 * s)
    leg_h = int(26 * s)
    arm_w = int(11 * s)
    arm_h = int(30 * s)
    body_top = cy - body_h

    # ── 气功光环 ──
    for i in range(6):
        r = int((60 - i * 8) * s)
        alpha = max(0, 40 - i * 5)
        if alpha > 0:
            draw.ellipse(
                [cx - r, body_top - int(5*s) - r, cx + r, body_top - int(5*s) + r],
                outline=(255, 200, 50, alpha), width=int(1.5*s))

    # ── 腿 ──
    for side in [-1, 1]:
        lx = cx + int(side * 11 * s)
        # 裤子
        rect(draw, [lx - leg_w//2, body_top + body_h,
                     lx + leg_w//2, body_top + body_h + leg_h],
             robe_dark)
        # 白色绑腿
        for i in range(3):
            wy = body_top + body_h + leg_h - (i+1)*int(7*s)
            rect(draw, [lx - leg_w//2 - int(1*s), wy,
                         lx + leg_w//2 + int(1*s), wy + int(5*s)],
                 wrap)
        # 武僧布鞋（替代赤脚）
        foot_w = int(leg_w * 1.5)
        foot_h = int(8 * s)
        ellipse_box = [lx - foot_w//2, body_top + body_h + leg_h,
                       lx + foot_w//2, body_top + body_h + leg_h + foot_h]
        draw.ellipse(ellipse_box, fill=shoe, outline=OUTLINE, width=1)
        # 鞋头
        draw.ellipse([lx - foot_w//2 + int(2*s), body_top + body_h + leg_h,
                       lx + foot_w//2 - int(2*s), body_top + body_h + leg_h + int(3*s)],
                     fill=shoe_dark)

    # ── 身体（交领武僧袍+云纹）──
    rect(draw, [cx - body_w//2, body_top, cx + body_w//2, body_top + body_h],
         robe, OUTLINE, 2)
    # 交领
    collar_w = int(10 * s)
    collar_h = int(18 * s)
    draw.polygon([(cx - collar_w, body_top), (cx, body_top + collar_h), (cx + collar_w, body_top)],
                 fill=wrap)
    # 云纹暗纹（袍子上）
    for i in range(2):
        cloud_y = body_top + int(body_h * 0.3) + i * int(body_h * 0.25)
        cloud_x = cx - int(8*s) + i * int(16*s)
        circ(draw, cloud_x, cloud_y, int(3*s), robe_light)
        circ(draw, cloud_x + int(4*s), cloud_y - int(1*s), int(2*s), robe_light)

    # 红腰带
    belt_y = body_top + int(body_h * 0.65)
    rect(draw, [cx - body_w//2 - int(2*s), belt_y,
                 cx + body_w//2 + int(2*s), belt_y + int(9*s)],
         belt, OUTLINE, 1)
    # 腰带结+飘带
    knot_y = belt_y + int(4*s)
    circ(draw, cx, knot_y, int(6*s), belt, OUTLINE, 1)
    draw.line([cx, knot_y, cx - int(14*s), knot_y + int(18*s)], fill=belt, width=int(3*s))
    draw.line([cx, knot_y, cx + int(14*s), knot_y + int(18*s)], fill=belt, width=int(3*s))

    # ── 袈裟披肩（金色半透明，搭在左肩）──
    cassock_pts = [
        (cx - body_w//2 - int(5*s), body_top + int(5*s)),
        (cx - body_w//2 + int(15*s), body_top + int(5*s)),
        (cx - body_w//2 + int(20*s), body_top + body_h - int(10*s)),
        (cx - body_w//2 - int(10*s), body_top + body_h - int(5*s)),
    ]
    draw.polygon(cassock_pts, fill=(220, 180, 50, 50))
    # 袈裟边纹
    draw.line([cassock_pts[0][0], cassock_pts[0][1], cassock_pts[3][0], cassock_pts[3][1]],
              fill=(200, 160, 30), width=int(2*s))

    # ── 手臂（灯笼袖+金色护手）──
    for side in [-1, 1]:
        ax = cx + int(side * (body_w//2 + arm_w//2 + 2*s))
        # 灯笼袖
        sleeve_w = int(arm_w * 1.4)
        sleeve_h = int(arm_h * 0.55)
        rect(draw, [ax - sleeve_w//2, body_top + int(6*s),
                     ax + sleeve_w//2, body_top + int(6*s) + sleeve_h],
             robe, OUTLINE, 1)
        # 袖口
        draw.line([ax - sleeve_w//2, body_top + int(6*s) + sleeve_h,
                   ax + sleeve_w//2, body_top + int(6*s) + sleeve_h],
                  fill=robe_dark, width=int(2*s))
        # 前臂
        forearm_y = body_top + int(6*s) + sleeve_h
        forearm_h = int(arm_h * 0.3)
        rect(draw, [ax - int(arm_w*0.6)//2, forearm_y,
                     ax + int(arm_w*0.6)//2, forearm_y + forearm_h],
             skin)
        # ★ 金色护手（替代布护腕）
        guard_y = forearm_y + forearm_h
        guard_h = int(arm_h * 0.35)
        rect(draw, [ax - int(arm_w*0.7), guard_y,
                     ax + int(arm_w*0.7), guard_y + guard_h],
             guard, guard_dark, 2)
        # 护手纹路
        draw.line([ax - int(arm_w*0.5), guard_y + int(3*s),
                   ax + int(arm_w*0.5), guard_y + int(3*s)],
                  fill=guard_dark, width=int(1*s))
        draw.line([ax - int(arm_w*0.5), guard_y + guard_h - int(3*s),
                   ax + int(arm_w*0.5), guard_y + guard_h - int(3*s)],
                  fill=guard_dark, width=int(1*s))
        # 拳头
        fist_y = guard_y + guard_h + int(3*s)
        circ(draw, ax, fist_y, int(7*s), skin, OUTLINE, 2)
        # 金色流光
        circ(draw, ax, fist_y, int(10*s), None, outline=(255, 215, 60, 30), width=1)

    # ── 念珠（少量，挂在脖子上）──
    for i in range(4):
        angle = math.pi * 0.15 + i * math.pi * 0.17
        bx = int(cx + math.cos(angle) * 22 * s)
        by = int(body_top - int(2*s) + math.sin(angle) * 9 * s)
        br = int(6 * s)
        circ(draw, bx, by, br, (160, 120, 40), OUTLINE, 1)
        circ(draw, bx - int(2*s), by - int(2*s), int(2*s), (220, 180, 60))

    # ── 头部（短发武僧头，非光头）──
    head_y = body_top - head_r
    # 头皮底色
    circ(draw, cx, head_y, head_r, skin, OUTLINE, 3)
    # ★ 短发/寸头（深色发茬）
    hair_pts_top = []
    for angle_deg in range(200, 340, 10):
        angle = math.radians(angle_deg)
        hx = int(cx + math.cos(angle) * (head_r - int(2*s)))
        hy = int(head_y + math.sin(angle) * (head_r - int(2*s)))
        hair_pts_top.append((hx, hy))
    if len(hair_pts_top) > 2:
        draw.polygon(hair_pts_top, fill=hair)

    # ★ 小发髻（头顶正中）
    bun_r = int(8 * s)
    bun_y = head_y - head_r + int(2*s)
    circ(draw, cx, bun_y, bun_r, hair, OUTLINE, 1)
    # 发髻高光
    circ(draw, cx - int(2*s), bun_y - int(2*s), int(3*s), (50, 45, 45))

    # ★ 额饰（金色小圆片，3个）
    for i in range(3):
        ex = cx + (i - 1) * int(8 * s)
        ey = head_y - int(head_r * 0.45)
        circ(draw, ex, ey, int(4*s), gold, (180, 150, 30), 1)

    # 红头带
    band_r = head_r + int(2*s)
    draw.arc([cx - band_r, head_y - band_r, cx + band_r, head_y + band_r],
             200, 340, fill=headband, width=int(7*s))
    # 飘带
    rb_x = cx + int(band_r * 0.85)
    rb_y = head_y - int(band_r * 0.3)
    draw.polygon([
        (rb_x, rb_y), (rb_x + int(22*s), rb_y - int(5*s)),
        (rb_x + int(38*s), rb_y + int(8*s)), (rb_x + int(28*s), rb_y + int(15*s))
    ], fill=headband)
    draw.polygon([
        (rb_x + int(5*s), rb_y + int(3*s)), (rb_x + int(28*s), rb_y + int(12*s)),
        (rb_x + int(42*s), rb_y + int(5*s)), (rb_x + int(32*s), rb_y - int(2*s))
    ], fill=(180, 35, 35))

    # 眼睛
    for side in [-1, 1]:
        ex = cx + int(side * 11 * s)
        ey = head_y + int(3*s)
        draw.ellipse([ex - int(7*s), ey - int(6*s), ex + int(7*s), ey + int(6*s)],
                     fill=(255, 255, 255), outline=OUTLINE, width=1)
        circ(draw, ex + int(2*s), ey, int(4*s), (20, 20, 30))
        circ(draw, ex + int(3*s), ey - int(2*s), int(1.8*s), (255, 255, 255))
        # 眉毛（粗，上扬）
        draw.line([ex - int(8*s), ey - int(8*s), ex + int(7*s), ey - int(10*s)],
                  fill=hair, width=int(3*s))

    # 嘴巴
    draw.arc([cx - int(7*s), head_y + int(12*s) - int(5*s),
              cx + int(7*s), head_y + int(12*s) + int(5*s)],
             10, 170, fill=OUTLINE, width=int(2*s))


# ════════════════════════════════════════════════════════════
# 糖糖 v3 — 双环望仙髻+齐胸襦裙+披帛+水袖+翘头绣鞋
# ════════════════════════════════════════════════════════════
def draw_tangtang_v3(draw, cx, cy, s):
    robe = (230, 110, 160)
    robe_dark = (190, 80, 130)
    robe_edge = (100, 220, 200)
    hat = (90, 50, 120)
    hat_star = (255, 220, 80)
    boot = (140, 90, 170)
    hair = (240, 160, 180)
    skin = (255, 232, 210)

    head_r = int(30 * s)
    body_w = int(36 * s)
    body_h = int(40 * s)
    leg_w = int(11 * s)
    leg_h = int(24 * s)
    arm_w = int(10 * s)
    arm_h = int(28 * s)
    body_top = cy - body_h

    # ─ 法阵光环 ──
    for i in range(5):
        r = int((55 - i * 8) * s)
        alpha = max(0, 35 - i * 5)
        if alpha > 0:
            draw.ellipse(
                [cx - r, body_top - int(5*s) - r, cx + r, body_top - int(5*s) + r],
                outline=(230, 110, 200, alpha), width=int(1*s))

    # ─ 腿+翘头绣鞋 ──
    for side in [-1, 1]:
        lx = cx + int(side * 10 * s)
        rect(draw, [lx - leg_w//2, body_top + body_h,
                     lx + leg_w//2, body_top + body_h + int(leg_h*0.4)],
             robe_dark)
        # ★ 翘头绣花鞋
        boot_w = int(leg_w * 1.6)
        boot_h = int(leg_h * 0.7)
        boot_y = body_top + body_h + int(leg_h * 0.3)
        rect(draw, [lx - boot_w//2, boot_y, lx + boot_w//2, boot_y + boot_h],
             boot, OUTLINE, 1)
        # 翘头（鞋尖上翘）
        tip_dir = side
        draw.line([lx + int(tip_dir * boot_w//2), boot_y,
                   lx + int(tip_dir * (boot_w//2 + 8*s)), boot_y - int(5*s)],
                  fill=boot, width=int(3*s))
        # 绣花（小圆点装饰）
        circ(draw, lx, boot_y + int(boot_h * 0.4), int(2.5*s), (255, 200, 220))

    # ── 身体（齐胸襦裙）──
    # 下裙（A字形，渐变）
    skirt_w_top = int(body_w * 0.85)
    skirt_w_bot = int(body_w * 1.4)
    skirt_pts = [
        (cx - skirt_w_top//2, body_top + int(body_h*0.45)),
        (cx + skirt_w_top//2, body_top + int(body_h*0.45)),
        (cx + skirt_w_bot//2, body_top + body_h),
        (cx - skirt_w_bot//2, body_top + body_h),
    ]
    draw.polygon(skirt_pts, fill=robe)
    # 裙边
    draw.line([cx - skirt_w_bot//2, body_top + body_h - int(3*s),
               cx + skirt_w_bot//2, body_top + body_h - int(3*s)],
              fill=robe_edge, width=int(4*s))
    # 上身（齐胸）
    rect(draw, [cx - body_w//2, body_top, cx + body_w//2, body_top + int(body_h*0.5)],
         robe, OUTLINE, 1)
    # 交领
    collar_w = int(8*s)
    collar_h = int(14*s)
    draw.polygon([(cx - collar_w, body_top + int(3*s)),
                  (cx, body_top + int(3*s) + collar_h),
                  (cx + collar_w, body_top + int(3*s))],
                 fill=robe_edge)

    # ★ 炼丹围裙 → 改为腰间玉佩+丝带
    pendant_y = body_top + int(body_h * 0.55)
    # 丝带
    draw.line([cx - int(15*s), pendant_y, cx + int(15*s), pendant_y],
              fill=robe_edge, width=int(3*s))
    # 玉佩
    circ(draw, cx, pendant_y + int(5*s), int(5*s), (180, 240, 220), (100, 200, 180), 1)

    # ── 披帛（半透明薄荷绿，从肩部垂下）──
    for side in [-1, 1]:
        sx = cx + int(side * (body_w//2 - int(5*s)))
        # 披帛曲线（从肩到腰）
        sash_pts = [
            (sx, body_top + int(8*s)),
            (sx + int(side * 25*s), body_top + int(body_h * 0.3)),
            (sx + int(side * 30*s), body_top + int(body_h * 0.6)),
            (sx + int(side * 15*s), body_top + int(body_h * 0.8)),
        ]
        draw.polygon(sash_pts, fill=(100, 220, 200, 50))
        draw.line(sash_pts[0] + sash_pts[1], fill=(80, 200, 180), width=int(2*s))

    # ── 手臂（水袖）──
    for side in [-1, 1]:
        ax = cx + int(side * (body_w//2 + arm_w//2 + 3*s))
        # 水袖（宽大，下垂）
        sw = int(arm_w * 1.8)
        sh = int(arm_h * 0.65)
        rect(draw, [ax - sw//2, body_top + int(6*s),
                     ax + sw//2, body_top + int(6*s) + sh],
             robe, OUTLINE, 1)
        # 袖口边
        draw.line([ax - sw//2, body_top + int(6*s) + sh,
                   ax + sw//2, body_top + int(6*s) + sh],
                  fill=robe_edge, width=int(3*s))
        # 手（纤小，从水袖中伸出）
        hand_y = body_top + int(6*s) + sh + int(6*s)
        circ(draw, ax, hand_y, int(4.5*s), skin, OUTLINE, 1)

    # ── 法杖（拂尘+葫芦）──
    staff_x = cx + int((body_w//2 + arm_w * 1.8))
    staff_top = body_top - int(35 * s)
    staff_bot = body_top + body_h + leg_h
    # 竹节杖柄
    draw.line([staff_x, staff_top, staff_x, staff_bot], fill=(120, 80, 50), width=int(4*s))
    for i in range(6):
        ny = staff_top + i * int((staff_bot - staff_top) / 6)
        draw.line([staff_x - int(3*s), ny, staff_x + int(3*s), ny],
                  fill=(90, 60, 35), width=int(1*s))
    # 葫芦（杖顶）
    gourd_y = staff_top - int(12*s)
    circ(draw, staff_x, gourd_y + int(5*s), int(14*s), (200, 60, 60), OUTLINE, 2)
    circ(draw, staff_x, gourd_y - int(8*s), int(10*s), (220, 80, 80), OUTLINE, 2)
    rect(draw, [staff_x - int(4*s), gourd_y - int(18*s),
                 staff_x + int(4*s), gourd_y - int(14*s)],
         (180, 150, 50), OUTLINE, 1)
    # 丹气
    for i in range(3):
        puff_x = staff_x + int((i - 1) * 6 * s)
        puff_y = gourd_y - int(22*s) - i * int(5*s)
        circ(draw, puff_x, puff_y, int((4-i)*s), [(255,180,200,100),(180,220,255,80),(200,255,200,60)][i])

    # ─ ★ 双环望仙髻（替代道冠）──
    head_y = body_top - head_r
    circ(draw, cx, head_y, head_r, skin, OUTLINE, 2)

    # 发髻左（大圆环）
    bun_l_x = cx - int(22*s)
    bun_l_y = head_y - int(head_r * 0.75)
    bun_r_size = int(14 * s)
    circ(draw, bun_l_x, bun_l_y, bun_r_size, hair, OUTLINE, 1)
    # 发髻中心（空心环感）
    circ(draw, bun_l_x, bun_l_y, int(6*s), skin)
    circ(draw, bun_l_x, bun_l_y, int(6*s), None, outline=OUTLINE, width=1)

    # 发髻右
    bun_r_x = cx + int(22*s)
    bun_r_y = head_y - int(head_r * 0.75)
    circ(draw, bun_r_x, bun_r_y, bun_r_size, hair, OUTLINE, 1)
    circ(draw, bun_r_x, bun_r_y, int(6*s), skin)
    circ(draw, bun_r_x, bun_r_y, int(6*s), None, outline=OUTLINE, width=1)

    # 发髻装饰（金色发簪穿过两髻）
    draw.line([bun_l_x - int(8*s), bun_l_y, bun_r_x + int(8*s), bun_r_y],
              fill=hat_star, width=int(3*s))
    # 发簪两端珠饰
    circ(draw, bun_l_x - int(8*s), bun_l_y, int(3*s), hat_star)
    circ(draw, bun_r_x + int(8*s), bun_r_y, int(3*s), hat_star)

    # 发带（从发髻垂下）
    draw.line([bun_l_x, bun_l_y + bun_r_size, bun_l_x - int(10*s), bun_l_y + bun_r_size + int(20*s)],
              fill=robe_edge, width=int(2*s))
    draw.line([bun_r_x, bun_r_y + bun_r_size, bun_r_x + int(10*s), bun_r_y + bun_r_size + int(20*s)],
              fill=robe_edge, width=int(2*s))

    # 刘海
    for i in range(3):
        bx = cx + (i - 1) * int(9*s)
        by = head_y - int(head_r * 0.45)
        circ(draw, bx, by, int(8*s), hair)

    # ★ 花钿（额头装饰）
    circ(draw, cx, head_y - int(head_r * 0.35), int(3*s), (255, 80, 100))

    # 眼睛（杏眼）
    for side in [-1, 1]:
        ex = cx + int(side * 10 * s)
        ey = head_y + int(2*s)
        draw.ellipse([ex - int(7*s), ey - int(6*s), ex + int(7*s), ey + int(6*s)],
                     fill=(255, 255, 255), outline=OUTLINE, width=1)
        circ(draw, ex + int(1*s), ey, int(4*s), (200, 60, 120))
        circ(draw, ex + int(2*s), ey - int(2*s), int(1.8*s), (255, 255, 255))
        # 柳叶眉
        draw.line([ex - int(7*s), ey - int(9*s), ex + int(6*s), ey - int(8*s)],
                  fill=(40, 30, 35), width=int(2*s))

    # 樱桃小口
    circ(draw, cx, head_y + int(12*s), int(3*s), (200, 80, 100))
    # 腮红
    circ(draw, cx - int(15*s), head_y + int(6*s), int(5*s), (255, 160, 160, 60))
    circ(draw, cx + int(15*s), head_y + int(6*s), int(5*s), (255, 160, 160, 60))


# ════════════════════════════════════════════════════════════
# 小鬼丸 v3 — 粗壮弯角+獠牙+狼牙棒+小尾巴+兽面肩甲
# ════════════════════════════════════════════════════════════
def draw_kiguemaru_v3(draw, cx, cy, s):
    skin = (170, 150, 210)
    skin_dark = (130, 110, 175)
    belly = (180, 35, 35)
    belly_dark = (140, 25, 25)
    armor = (70, 70, 80)
    armor_hi = (100, 100, 110)
    guard = (40, 40, 50)
    horn = (220, 40, 50)
    hammer_body = (200, 50, 60)
    hammer_gold = (220, 180, 50)
    hammer_blue = (80, 180, 220)
    hammer_handle = (100, 60, 40)
    shorts = (70, 70, 80)
    bell = (255, 200, 50)
    eye_gold = (255, 210, 60)
    fang = (240, 235, 220)

    head_r = int(30 * s)
    body_w = int(44 * s)
    body_h = int(40 * s)
    leg_w = int(15 * s)
    leg_h = int(24 * s)
    arm_w = int(13 * s)
    arm_h = int(28 * s)
    body_top = cy - body_h

    # ── 腿 ──
    for side in [-1, 1]:
        lx = cx + int(side * 13 * s)
        rect(draw, [lx - leg_w//2, body_top + body_h,
                     lx + leg_w//2, body_top + body_h + int(leg_h*0.45)],
             shorts, OUTLINE, 1)
        rect(draw, [lx - leg_w//2, body_top + body_h + int(leg_h*0.45),
                     lx + leg_w//2, body_top + body_h + leg_h],
             skin)
        foot_w = int(leg_w * 1.4)
        foot_h = int(8 * s)
        draw.ellipse([lx - foot_w//2, body_top + body_h + leg_h,
                       lx + foot_w//2, body_top + body_h + leg_h + foot_h],
                     fill=skin_dark, outline=OUTLINE, width=2)

    # ── ★ 小尾巴（从身后伸出）──
    tail_base_x = cx + int(15*s)
    tail_base_y = body_top + int(body_h * 0.85)
    tail_pts = [
        (tail_base_x, tail_base_y),
        (tail_base_x + int(25*s), tail_base_y - int(10*s)),
        (tail_base_x + int(35*s), tail_base_y - int(25*s)),
        (tail_base_x + int(30*s), tail_base_y - int(30*s)),
        (tail_base_x + int(20*s), tail_base_y - int(15*s)),
        (tail_base_x + int(5*s), tail_base_y + int(5*s)),
    ]
    draw.polygon(tail_pts, fill=skin, outline=OUTLINE, width=1)
    # 尾巴尖端（火焰/心形）
    tip_x = tail_base_x + int(33*s)
    tip_y = tail_base_y - int(28*s)
    # 心形简化
    circ(draw, tip_x - int(3*s), tip_y, int(4*s), (220, 40, 50))
    circ(draw, tip_x + int(3*s), tip_y, int(4*s), (220, 40, 50))
    draw.polygon([(tip_x - int(7*s), tip_y + int(2*s)),
                  (tip_x, tip_y + int(10*s)),
                  (tip_x + int(7*s), tip_y + int(2*s))],
                 fill=(220, 40, 50))

    # ── 身体（肚兜+云纹）──
    rect(draw, [cx - body_w//2, body_top, cx + body_w//2, body_top + body_h],
         belly, OUTLINE, 3)
    rect(draw, [cx - body_w//2, body_top + body_h//2,
                 cx + body_w//2, body_top + body_h],
         belly_dark)
    draw.line([cx - body_w//2, body_top + int(5*s),
               cx + body_w//2, body_top + int(5*s)],
              fill=belly_dark, width=int(3*s))
    draw.line([cx, body_top, cx, body_top + body_h],
              fill=belly_dark, width=int(3*s))
    # 云纹
    for i in range(2):
        cx2 = cx + (i * 2 - 1) * int(12*s)
        cy2 = body_top + int(body_h * 0.7)
        circ(draw, cx2, cy2, int(3*s), (200, 60, 60, 100))
        circ(draw, cx2 + int(3*s), cy2 - int(1*s), int(2*s), (200, 60, 60, 80))

    # 绳腰带
    rope_y = body_top + int(body_h * 0.88)
    rect(draw, [cx - int(body_w*0.65), rope_y,
                 cx + int(body_w*0.65), rope_y + int(7*s)],
         (160, 130, 100), OUTLINE, 1)
    circ(draw, cx, rope_y + int(3*s), int(4*s), (140, 110, 80))

    # ── ★ 兽面大肩甲（更大更威武）──
    # 左大肩甲
    lsx = cx - body_w//2 - int(8*s)
    armor_w = int(20*s)
    armor_h = int(22*s)
    rect(draw, [lsx - armor_w//2, body_top - int(8*s),
                 lsx + armor_w//2, body_top - int(8*s) + armor_h],
         armor, OUTLINE, 2)
    # 兽面纹
    circ(draw, lsx, body_top - int(8*s) + int(8*s), int(7*s), armor_hi)
    circ(draw, lsx - int(3*s), body_top - int(8*s) + int(6*s), int(2*s), (30, 30, 30))
    circ(draw, lsx + int(3*s), body_top - int(8*s) + int(6*s), int(2*s), (30, 30, 30))
    # 獠牙装饰（肩甲下方）
    for ti in range(3):
        tx = lsx + (ti - 1) * int(4*s)
        draw.line([tx, body_top - int(8*s) + armor_h - int(2*s),
                   tx, body_top - int(8*s) + armor_h + int(5*s)],
                  fill=(220, 220, 210), width=int(2*s))
    # 右小肩甲
    rsx = cx + body_w//2 + int(6*s)
    rect(draw, [rsx - int(11*s), body_top + int(2*s),
                 rsx + int(11*s), body_top + int(16*s)],
         armor, OUTLINE, 2)

    # ── 手臂+铁指虎 ─
    for side in [-1, 1]:
        ax = cx + int(side * (body_w//2 + arm_w//2 + 6*s))
        rect(draw, [ax - arm_w//2, body_top + int(8*s),
                     ax + arm_w//2, body_top + int(8*s) + int(arm_h*0.5)],
             skin)
        # 铁护臂
        guard_y = body_top + int(8*s) + int(arm_h*0.5)
        guard_h = int(arm_h * 0.5)
        rect(draw, [ax - int(arm_w*0.6), guard_y,
                     ax + int(arm_w*0.6), guard_y + guard_h],
             guard, OUTLINE, 2)
        # ★ 铁指虎（拳头上的金属武器）
        fist_y = guard_y + guard_h + int(3*s)
        circ(draw, ax, fist_y, int(8*s), skin, OUTLINE, 2)
        # 指虎纹
        for fi in range(4):
            fa = fi * math.pi / 2
            fx = int(ax + math.cos(fa) * 6 * s)
            fy = int(fist_y + math.sin(fa) * 6 * s)
            circ(draw, fx, fy, int(2*s), (80, 80, 90))

    # ── ★ 狼牙棒（替代菱形锤）──
    hammer_cx_pos = cx + int((body_w//2 + arm_w + 30*s))
    hammer_top = body_top - int(45 * s)
    hammer_bot = body_top + body_h + int(10*s)
    # 柄
    draw.line([hammer_cx_pos, hammer_top, hammer_cx_pos, hammer_bot],
              fill=hammer_handle, width=int(5*s))
    # 缠布
    for i in range(10):
        ty = hammer_top + int(25*s) + i * int(7*s)
        if ty < hammer_bot - int(10*s):
            draw.line([hammer_cx_pos - int(4*s), ty,
                       hammer_cx_pos + int(4*s), ty + int(4*s)],
                      fill=(180, 50, 50), width=int(2*s))
    # 棒头（圆柱+狼牙）
    head_w = int(16 * s)
    head_h = int(30 * s)
    head_cy = hammer_top - int(10*s)
    # 棒身
    rect(draw, [hammer_cx_pos - head_w, head_cy - head_h//2,
                 hammer_cx_pos + head_w, head_cy + head_h//2],
         hammer_body, OUTLINE, 2)
    # 金色环纹
    draw.line([hammer_cx_pos - head_w, head_cy,
               hammer_cx_pos + head_w, head_cy],
              fill=hammer_gold, width=int(3*s))
    draw.line([hammer_cx_pos - head_w, head_cy - int(8*s),
               hammer_cx_pos + head_w, head_cy - int(8*s)],
              fill=hammer_blue, width=int(2*s))
    # 狼牙（周围突出的尖刺）
    for ai in range(8):
        angle = ai * math.pi / 4
        spike_x = int(hammer_cx_pos + math.cos(angle) * (head_w + int(6*s)))
        spike_y = int(head_cy + math.sin(angle) * (head_h//2 + int(4*s)))
        # 只在上下左右方向有牙
        if ai % 2 == 0:
            base_x = int(hammer_cx_pos + math.cos(angle) * head_w)
            base_y = int(head_cy + math.sin(angle) * head_h//2)
            draw.line([base_x, base_y, spike_x, spike_y],
                      fill=(200, 200, 190), width=int(3*s))
    # 顶端宝珠
    circ(draw, hammer_cx_pos, hammer_top - int(8*s), int(7*s),
         hammer_gold, OUTLINE, 1)

    # ── 铃铛+骷髅挂饰 ──
    circ(draw, cx, body_top - int(4*s), int(5*s), bell, OUTLINE, 1)
    # 小骷髅
    skull_x = cx + int(12*s)
    skull_y = body_top + int(2*s)
    circ(draw, skull_x, skull_y, int(4*s), (220, 215, 200), OUTLINE, 1)
    circ(draw, skull_x - int(1.5*s), skull_y - int(1*s), int(1*s), (40, 40, 40))
    circ(draw, skull_x + int(1.5*s), skull_y - int(1*s), int(1*s), (40, 40, 40))

    # ── 头部 ──
    head_y = body_top - head_r
    circ(draw, cx, head_y, head_r, skin, OUTLINE, 3)

    # ★ 粗壮弯角（向后弯）
    for side in [-1, 1]:
        hx = cx + int(side * 14 * s)
        # 角根部（粗）
        draw.line([hx, head_y - int(head_r * 0.5),
                   hx + int(side * 5*s), head_y - int(head_r * 1.1)],
                  fill=horn, width=int(9*s))
        # 角中段（弯）
        draw.line([hx + int(side * 5*s), head_y - int(head_r * 1.1),
                   hx + int(side * 12*s), head_y - int(head_r * 1.3)],
                  fill=horn, width=int(7*s))
        # 角尖（细，向后弯）
        draw.line([hx + int(side * 12*s), head_y - int(head_r * 1.3),
                   hx + int(side * 18*s), head_y - int(head_r * 1.1)],
                  fill=horn, width=int(5*s))
        # 角纹路
        for ri in range(3):
            ry = head_y - int(head_r * 0.6) - ri * int(head_r * 0.25)
            rx = hx + int(side * ri * 3 * s)
            draw.line([rx - int(4*s), ry, rx + int(4*s), ry],
                      fill=(180, 30, 40), width=int(1*s))
        # 角尖发光
        tip_x = hx + int(side * 18*s)
        tip_y = head_y - int(head_r * 1.1)
        circ(draw, tip_x, tip_y, int(5*s), (255, 150, 60, 100))

    # 尖耳朵
    for side in [-1, 1]:
        ex = cx + int(side * head_r * 0.9)
        ear_pts = [
            (ex, head_y - int(3*s)),
            (ex + int(side * 12*s), head_y - int(8*s)),
            (ex + int(side * 5*s), head_y + int(5*s))
        ]
        draw.polygon(ear_pts, fill=skin, outline=OUTLINE)

    # ★ 獠牙（从嘴角露出）
    for side in [-1, 1]:
        fx = cx + int(side * 6 * s)
        fy = head_y + int(13*s)
        # 上獠牙
        draw.line([fx, fy, fx + int(side*2*s), fy + int(6*s)],
                  fill=fang, width=int(2.5*s*0.4))

    # 眼睛（怒目）
    for side in [-1, 1]:
        ex = cx + int(side * 11 * s)
        ey = head_y + int(1*s)
        draw.ellipse([ex - int(8*s), ey - int(7*s), ex + int(8*s), ey + int(7*s)],
                     fill=(255, 255, 235), outline=OUTLINE, width=1)
        circ(draw, ex + int(2*s), ey, int(4.5*s), eye_gold)
        circ(draw, ex + int(2*s), ey, int(4.5*s), (200, 160, 30), 1)
        circ(draw, ex + int(3*s), ey, int(2*s), (20, 20, 20))
        circ(draw, ex + int(4*s), ey - int(2*s), int(1.5*s), (255, 255, 255))
        draw.line([ex - int(8*s), ey - int(8*s), ex + int(8*s), ey - int(10*s)],
                  fill=(30, 25, 30), width=int(4*s))

    # 锯齿嘴
    mouth_y = head_y + int(13*s)
    for i in range(5):
        mx = cx + int((i * 4 - 8) * s)
        if i % 2 == 0:
            draw.line([mx, mouth_y, mx + int(2*s), mouth_y + int(4*s)],
                      fill=OUTLINE, width=int(2*s))
        else:
            draw.line([mx, mouth_y + int(4*s), mx + int(2*s), mouth_y],
                      fill=OUTLINE, width=int(2*s))


# ════════════════════════════════════════════════════════════
# 主渲染
# ════════════════════════════════════════════════════════════
def render():
    img = Image.new('RGBA', (W, H), (*BG_DARK, 255))
    draw = ImageDraw.Draw(img)

    # 背景
    for y in range(120):
        alpha = int((120 - y) / 120 * 100)
        draw.line([(0, y), (W, y)], fill=(0, 0, 0, alpha))
    draw.line([(20, 0), (20, H)], fill=(60, 50, 40, 80), width=2)
    draw.line([(W-20, 0), (W-20, H)], fill=(60, 50, 40, 80), width=2)
    draw.line([(0, 90), (W, 90)], fill=(80, 70, 50, 60), width=1)

    # 萤火
    for i in range(30):
        px = (i * 173 + 50) % W
        py = (i * 127 + 30) % int(H * 0.75)
        sz = 1 + (i * 5) % 3
        al = 15 + (i * 11) % 30
        draw.ellipse([px - sz, py - sz, px + sz, py + sz], fill=(200, 200, 255, al))

    # 地面
    floor_y = CHAR_Y + 12
    for y_off in range(H - floor_y):
        ratio = y_off / (H - floor_y)
        r = int(35 + ratio * 10)
        g = int(35 + ratio * 10)
        b = int(55 + ratio * 10)
        draw.line([(0, floor_y + y_off), (W, floor_y + y_off)], fill=(r, g, b))
    draw.line([(0, floor_y), (W, floor_y)], fill=(80, 80, 100), width=2)
    for x in range(0, W, 80):
        draw.line([(x, floor_y), (x, H)], fill=(45, 45, 65), width=1)
    for yo in [40, 80, 120, 160]:
        draw.line([(0, floor_y + yo), (W, floor_y + yo)], fill=(45, 45, 65), width=1)

    # 标题
    txt(draw, "格斗萌主 — 角色视觉定稿 v3.0", W//2, 20, size=36, fill=(240, 235, 220))
    txt(draw, "Q版2D横版格斗 · 三头身 · 糖果炸裂风 · 国风 · 梦幻西游参考", W//2, 65, size=20, fill=(180, 175, 160))
    draw.line([W//2 - 350, 95, W//2 + 350, 95], fill=(120, 100, 70), width=1)
    circ(draw, W//2 - 350, 95, 3, (180, 150, 80))
    circ(draw, W//2 + 350, 95, 3, (180, 150, 80))

    # 分隔线
    for dx in [CHAR_1_X + 220, CHAR_2_X - 220, CHAR_2_X + 220, CHAR_3_X - 220]:
        for dy in range(CHAR_Y - 200, CHAR_Y + 200, 8):
            alpha = int(40 * (1 - abs(dy - CHAR_Y) / 200))
            if alpha > 0:
                draw.line([(dx, dy), (dx, dy + 4)], fill=(80, 80, 100, alpha), width=1)

    # 角色
    s = SCALE
    draw_huikong_v3(draw, CHAR_1_X, CHAR_Y, s)
    draw_tangtang_v3(draw, CHAR_2_X, CHAR_Y, s)
    draw_kiguemaru_v3(draw, CHAR_3_X, CHAR_Y, s)

    # 标签
    labels = [
        (CHAR_1_X, "气功小武僧", "慧空 · Blazing Monkfist", "近战连招 | 气功层数", (51, 102, 204)),
        (CHAR_2_X, "糖果魔导师", "糖糖 · Sugar Hexblade", "中程控场 | 糖果配方", (230, 110, 160)),
        (CHAR_3_X, "萌鬼狂战士", "小鬼丸 · Oni Crushkin", "近战爆发 | 怒气变身", (155, 89, 200)),
    ]
    for lx, title, name, role, accent in labels:
        box_w, box_h = 280, 90
        bx = lx - box_w // 2
        by = CHAR_Y + 230
        draw.rectangle([bx, by, bx + box_w, by + box_h], fill=(20, 20, 35, 200))
        draw.rectangle([bx, by, bx + box_w, by + 5], fill=accent)
        circ(draw, bx+5, by+10, 2, accent)
        circ(draw, bx+box_w-5, by+10, 2, accent)
        txt(draw, title, lx, by + 15, size=22, fill=(240, 235, 220))
        txt(draw, name, lx, by + 42, size=16, fill=accent)
        txt(draw, role, lx, by + 65, size=14, fill=(160, 155, 145))

    txt(draw, "辨识度: 主色唯一 | 剪影唯一 | 32px可辨 | 特效色唯一 | 轮廓(圆/三角/方)",
        W//2, H - 40, size=14, fill=(130, 125, 115))

    out = "/tmp/first-game-characters/characters_v3_mhxy.png"
    os.makedirs(os.path.dirname(out), exist_ok=True)
    img.convert('RGB').save(out, 'PNG', quality=95)
    print(f"✅ 已保存: {out} ({os.path.getsize(out)} bytes)")
    return out

if __name__ == '__main__':
    render()

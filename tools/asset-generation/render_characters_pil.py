#!/usr/bin/env python3
"""格斗萌主 — 角色视觉定稿渲染 (PIL)"""

from PIL import Image, ImageDraw, ImageFont
import math
import os

W, H = 1280, 720
SCALE = 2.5

# Colors
BG_DARK = (26, 26, 46)        # #1A1A2E
BG_FLOOR = (45, 45, 68)       # #2D2D44
OUTLINE = (26, 26, 38)

# Character positions
CHAR_Y = 420
CHAR_1_X = 260   # 慧空
CHAR_2_X = 640   # 糖糖
CHAR_3_X = 1020  # 小鬼丸


def draw_text(draw, text, x, y, size=16, fill=(255, 255, 255)):
    """Draw centered text"""
    try:
        font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", size)
    except:
        try:
            font = ImageFont.truetype("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf", size)
        except:
            font = ImageFont.load_default()
    bbox = draw.textbbox((0, 0), text, font=font)
    tw = bbox[2] - bbox[0]
    draw.text((x - tw // 2, y), text, fill=fill, font=font)


def draw_text_cn(draw, text, x, y, size=18, fill=(255, 255, 255)):
    """Draw text, trying Chinese font"""
    cn_fonts = [
        "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
        "/usr/share/fonts/truetype/noto/NotoSansCJK-Regular.ttc",
        "/usr/share/fonts/truetype/noto/NotoSansSC-Regular.otf",
        "/usr/share/fonts/truetype/wqy/wqy-zenhei.ttc",
        "/usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf",
    ]
    font = None
    for fp in cn_fonts:
        if os.path.exists(fp):
            try:
                font = ImageFont.truetype(fp, size)
                break
            except:
                continue
    if font is None:
        try:
            font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", size)
        except:
            font = ImageFont.load_default()
    bbox = draw.textbbox((0, 0), text, font=font)
    tw = bbox[2] - bbox[0]
    draw.text((x - tw // 2, y), text, fill=fill, font=font)


def draw_ellipse_outline(draw, bbox, fill, outline=OUTLINE, width=2):
    draw.ellipse(bbox, fill=fill, outline=outline, width=width)


def draw_rect_outline(draw, xy, fill, outline=OUTLINE, width=2):
    draw.rectangle(xy, fill=fill, outline=outline, width=width)


# ════════════════════════════════════════════════════════════
# 角色1: 气功小武僧 · 慧空
# ════════════════════════════════════════════════════════════
def draw_huikong(draw, cx, cy, s):
    skin = (255, 212, 176)
    body_blue = (51, 102, 204)
    body_dark = (26, 51, 102)
    belt_red = (204, 51, 51)
    headband = (221, 51, 51)
    bead = (139, 105, 20)
    gold = (255, 215, 0)
    wrap_white = (240, 240, 238)
    guard = (68, 68, 68)

    head_r = int(28 * s)
    body_w = int(35 * s)
    body_h = int(40 * s)
    leg_w = int(12 * s)
    leg_h = int(25 * s)
    arm_w = int(10 * s)
    arm_h = int(30 * s)

    body_top = cy - body_h

    # 气功光环
    glow_alpha = 30
    for r in range(55, 30, -5):
        draw.ellipse(
            [cx - int(r*s), body_top - int(10*s) - int(r*s),
             cx + int(r*s), body_top - int(10*s) + int(r*s)],
            outline=(255, 215, 0, glow_alpha), width=1
        )

    # 腿
    for side in [-1, 1]:
        lx = cx + int(side * 10 * s)
        # 裤子
        draw_rect_outline(draw,
            [lx - leg_w//2, body_top + body_h, lx + leg_w//2, body_top + body_h + leg_h],
            body_blue, width=2)
        # 白色绑腿
        for i in range(3):
            wy = body_top + body_h + leg_h - (i+1)*int(6*s)
            draw.rectangle([lx - leg_w//2, wy, lx + leg_w//2, wy + int(4*s)], fill=wrap_white)
        # 赤脚
        draw_rect_outline(draw,
            [lx - int(leg_w*0.6), body_top + body_h + leg_h,
             lx + int(leg_w*0.6), body_top + body_h + leg_h + int(5*s)],
            skin, width=1)

    # 身体
    draw_rect_outline(draw,
        [cx - body_w//2, body_top, cx + body_w//2, body_top + body_h],
        body_blue, width=3)
    # V领
    draw.line([cx - int(8*s), body_top, cx, body_top + int(15*s)], fill=wrap_white, width=int(3*s))
    draw.line([cx + int(8*s), body_top, cx, body_top + int(15*s)], fill=wrap_white, width=int(3*s))
    # 红腰带
    draw_rect_outline(draw,
        [cx - body_w//2, body_top + int(body_h*0.6), cx + body_w//2, body_top + int(body_h*0.6) + int(8*s)],
        belt_red, width=2)
    draw.ellipse([cx - int(5*s), body_top + int(body_h*0.6) - int(2*s),
                  cx + int(5*s), body_top + int(body_h*0.6) + int(8*s)], fill=belt_red)

    # 手臂
    for side in [-1, 1]:
        ax = cx + int(side * (body_w//2 + arm_w//2))
        # 袖子
        draw_rect_outline(draw,
            [ax - arm_w//2, body_top + int(5*s), ax + arm_w//2, body_top + int(5*s) + int(arm_h*0.6)],
            body_blue, width=2)
        # 前臂
        draw.rectangle([ax - arm_w//2, body_top + int(5*s) + int(arm_h*0.6),
                        ax + arm_w//2, body_top + int(5*s) + arm_h], fill=skin)
        # 护腕
        draw_rect_outline(draw,
            [ax - arm_w//2 - int(1*s), body_top + int(5*s) + int(arm_h*0.5),
             ax + arm_w//2 + int(1*s), body_top + int(5*s) + int(arm_h*0.5) + int(5*s)],
            guard, width=1)
        # 拳头
        fist_y = body_top + int(5*s) + arm_h
        draw_ellipse_outline(draw,
            [ax - int(6*s), fist_y - int(6*s), ax + int(6*s), fist_y + int(6*s)],
            skin, width=2)
        # 金色绑带
        draw.arc([ax - int(6*s), fist_y - int(6*s), ax + int(6*s), fist_y + int(6*s)],
                 0, 360, fill=(204, 165, 26), width=int(2*s))
        # 金光
        draw.ellipse([ax - int(10*s), fist_y - int(10*s), ax + int(10*s), fist_y + int(10*s)],
                     outline=(255, 215, 0, 40), width=1)

    # 念珠
    for i in range(5):
        angle = math.pi * 0.15 + i * math.pi * 0.14
        bx = int(cx + math.cos(angle) * 22 * s)
        by = int(body_top - 2*s + math.sin(angle) * 8 * s)
        draw_ellipse_outline(draw,
            [bx - int(5*s), by - int(5*s), bx + int(5*s), by + int(5*s)],
            bead, width=1)
        # 金色光泽
        draw.ellipse([bx - int(2*s), by - int(2*s), bx + int(2*s), by + int(2*s)],
                     fill=(179, 140, 26, 128))

    # 头部
    head_y = body_top - head_r
    # 光头
    draw_ellipse_outline(draw,
        [cx - head_r, head_y - head_r, cx + head_r, head_y + head_r],
        skin, width=3)

    # 气功纹
    for i in range(3):
        vx = cx + (i - 1) * int(8 * s)
        vy = head_y - int(head_r * 0.6)
        draw.ellipse([vx - int(3*s), vy - int(3*s), vx + int(3*s), vy + int(3*s)],
                     fill=(255, 215, 0, 150))

    # 红头带
    draw.arc([cx - head_r, head_y - head_r, cx + head_r, head_y + head_r],
             144, 36, fill=headband, width=int(6*s))
    # 飘带
    ribbon_end_x = cx + int(head_r * 1.3)
    ribbon_end_y = head_y - int(head_r * 0.5)
    draw.line([cx + int(head_r*0.7), head_y - int(head_r*0.3),
               ribbon_end_x, ribbon_end_y], fill=headband, width=int(4*s))
    draw.line([ribbon_end_x, ribbon_end_y,
               cx + int(head_r*1.6), head_y - int(head_r*0.3)], fill=headband, width=int(3*s))

    # 眼睛
    for side in [-1, 1]:
        ex = cx + int(side * 10 * s)
        ey = head_y + int(2*s)
        # 眼白
        draw_ellipse_outline(draw,
            [ex - int(6*s), ey - int(6*s), ex + int(6*s), ey + int(6*s)],
            (255, 255, 255), width=1)
        # 瞳孔
        draw.ellipse([ex + int(2*s) - int(3.5*s), ey - int(3.5*s),
                      ex + int(2*s) + int(3.5*s), ey + int(3.5*s)],
                     fill=(26, 26, 51))
        # 高光
        draw.ellipse([ex + int(3*s) - int(1.5*s), ey - int(1*s) - int(1.5*s),
                      ex + int(3*s) + int(1.5*s), ey - int(1*s) + int(1.5*s)],
                     fill=(255, 255, 255))
        # 眉毛
        draw.line([ex - int(6*s), ey - int(7*s),
                   ex + int(6*s), ey - int(9*s)], fill=OUTLINE, width=int(3*s))

    # 嘴巴
    draw.arc([cx - int(6*s), head_y + int(12*s) - int(6*s),
              cx + int(6*s), head_y + int(12*s) + int(6*s)],
             12, 168, fill=OUTLINE, width=int(2.5*s*0.3))


# ════════════════════════════════════════════════════════════
# 角色2: 糖果魔导师 · 糖糖
# ════════════════════════════════════════════════════════════
def draw_tangtang(draw, cx, cy, s):
    robe_pink = (255, 107, 157)
    robe_edge = (0, 245, 212)
    hat_purple = (108, 52, 131)
    hat_star = (255, 215, 0)
    apron_white = (255, 248, 231)
    boot_purple = (155, 89, 182)
    hair_pink = (255, 182, 193)
    skin = (255, 228, 196)

    head_r = int(26 * s)
    body_w = int(32 * s)
    body_h = int(38 * s)
    leg_w = int(10 * s)
    leg_h = int(22 * s)
    arm_w = int(9 * s)
    arm_h = int(28 * s)

    body_top = cy - body_h

    # 魔法光环
    for r in range(50, 25, -5):
        draw.ellipse(
            [cx - int(r*s), body_top - int(10*s) - int(r*s),
             cx + int(r*s), body_top - int(10*s) + int(r*s)],
            outline=(255, 107, 157, 30), width=1)

    # 腿+靴子
    for side in [-1, 1]:
        lx = cx + int(side * 9 * s)
        # 小腿
        draw.rectangle([lx - leg_w//2, body_top + body_h,
                        lx + leg_w//2, body_top + body_h + int(leg_h*0.5)], fill=robe_pink)
        # 靴子
        draw_rect_outline(draw,
            [lx - int(leg_w*0.7), body_top + body_h + int(leg_h*0.5),
             lx + int(leg_w*0.7), body_top + body_h + leg_h],
            boot_purple, width=2)

    # 身体（魔法袍下摆）
    draw_rect_outline(draw,
        [cx - int(body_w*0.7), body_top + int(body_h*0.5),
         cx + int(body_w*0.7), body_top + body_h],
        robe_pink, width=2)
    # 薄荷绿边
    draw.rectangle([cx - int(body_w*0.7), body_top + int(body_h*0.95),
                    cx + int(body_w*0.7), body_top + int(body_h*0.95) + int(4*s)],
                   fill=robe_edge)
    # 上半身
    draw_rect_outline(draw,
        [cx - body_w//2, body_top, cx + body_w//2, body_top + int(body_h*0.6)],
        robe_pink, width=2)

    # 白围裙
    draw_rect_outline(draw,
        [cx - int(body_w*0.35), body_top + int(5*s),
         cx + int(body_w*0.35), body_top + int(5*s) + int(body_h*0.7)],
        apron_white, width=1)
    # 糖渍
    splat_colors = [(255, 102, 153), (102, 204, 255), (255, 215, 0), (128, 255, 128)]
    for i in range(4):
        sx = cx + (i * 7 - 10) * int(s)
        sy = body_top + (15 + i * 8) * int(s)
        draw.ellipse([sx - int(2.5*s), sy - int(2.5*s), sx + int(2.5*s), sy + int(2.5*s)],
                     fill=splat_colors[i])

    # 糖果腰带
    draw.rectangle([cx - int(body_w*0.6), body_top + int(body_h*0.5),
                    cx + int(body_w*0.6), body_top + int(body_h*0.5) + int(5*s)],
                   fill=(255, 105, 180))
    # 小瓶子
    for i in range(3):
        bx = cx + (i * 12 - 12) * int(s)
        by = body_top + int(body_h*0.5) + int(6*s)
        draw_rect_outline(draw,
            [bx - int(2*s), by, bx + int(2*s), by + int(6*s)],
            splat_colors[i], width=1)

    # 手臂
    for side in [-1, 1]:
        ax = cx + int(side * (body_w//2 + arm_w//2))
        # 宽袖
        draw_rect_outline(draw,
            [ax - int(arm_w*0.7), body_top + int(5*s),
             ax + int(arm_w*0.7), body_top + int(5*s) + int(arm_h*0.5)],
            robe_pink, width=1)
        # 薄荷绿袖口
        draw.rectangle([ax - int(arm_w*0.7), body_top + int(5*s) + int(arm_h*0.45),
                        ax + int(arm_w*0.7), body_top + int(5*s) + int(arm_h*0.45) + int(4*s)],
                       fill=robe_edge)
        # 手
        hand_y = body_top + int(5*s) + int(arm_h*0.6)
        draw_ellipse_outline(draw,
            [ax - int(5*s), hand_y - int(5*s), ax + int(5*s), hand_y + int(5*s)],
            skin, width=1)

    # 法杖
    staff_x = cx + int((body_w//2 + arm_w*1.5))
    staff_top = body_top - int(30 * s)
    staff_bottom = body_top + body_h + leg_h
    # 杖柄
    draw_rect_outline(draw,
        [staff_x - int(2.5*s), staff_top, staff_x + int(2.5*s), staff_bottom],
        (93, 64, 55), width=1)
    # 棒棒糖
    candy_r = int(20 * s)
    candy_cy = staff_top - candy_r // 2
    draw_ellipse_outline(draw,
        [staff_x - candy_r, candy_cy - candy_r, staff_x + candy_r, candy_cy + candy_r],
        (255, 107, 157), width=2)
    # 螺旋
    spiral_colors = [(255, 255, 102), (102, 204, 255), (255, 153, 204), (128, 255, 179)]
    for i in range(6):
        angle = i * math.pi / 3
        sx = int(staff_x + math.cos(angle) * candy_r * 0.6)
        sy = int(candy_cy + math.sin(angle) * candy_r * 0.6)
        draw.ellipse([sx - int(4*s), sy - int(4*s), sx + int(4*s), sy + int(4*s)],
                     fill=spiral_colors[i % 4])
    # 高光
    draw.ellipse([staff_x - int(5*s) - int(5*s), candy_cy - int(5*s) - int(5*s),
                  staff_x - int(5*s) + int(5*s), candy_cy - int(5*s) + int(5*s)],
                 fill=(255, 255, 255, 100))

    # 双马尾
    tail_wave = int(3 * s)
    # 左马尾
    for dy, r in [(10*s, 10*s), (18*s, 8*s), (25*s, 7*s)]:
        draw.ellipse([cx - int(22*s) - int(r), head_y_pos(draw, body_top, head_r) + int(dy) - int(r) + tail_wave,
                      cx - int(22*s) + int(r), head_y_pos(draw, body_top, head_r) + int(dy) + int(r) + tail_wave],
                     fill=hair_pink)
    # 右马尾
    for dy, r in [(10*s, 10*s), (18*s, 8*s), (25*s, 7*s)]:
        draw.ellipse([cx + int(22*s) - int(r), head_y_pos(draw, body_top, head_r) + int(dy) - int(r) - tail_wave,
                      cx + int(22*s) + int(r), head_y_pos(draw, body_top, head_r) + int(dy) + int(r) - tail_wave],
                     fill=hair_pink)

    # 头部
    head_y = body_top - head_r
    draw_ellipse_outline(draw,
        [cx - head_r, head_y - head_r, cx + head_r, head_y + head_r],
        skin, width=2)

    # 刘海
    draw.ellipse([cx - int(8*s) - int(8*s), head_y - int(head_r*0.5) - int(8*s),
                  cx - int(8*s) + int(8*s), head_y - int(head_r*0.5) + int(8*s)], fill=hair_pink)
    draw.ellipse([cx + int(8*s) - int(8*s), head_y - int(head_r*0.5) - int(8*s),
                  cx + int(8*s) + int(8*s), head_y - int(head_r*0.5) + int(8*s)], fill=hair_pink)
    draw.ellipse([cx - int(9*s), head_y - int(head_r*0.6) - int(9*s),
                  cx + int(9*s), head_y - int(head_r*0.6) + int(9*s)], fill=hair_pink)

    # 帽子
    # 帽檐
    draw_rect_outline(draw,
        [cx - int(head_r*1.3), head_y - int(head_r*0.3),
         cx + int(head_r*1.3), head_y - int(head_r*0.3) + int(6*s)],
        hat_purple, width=2)
    # 帽身三角
    hat_pts = [
        (cx - int(head_r*1.1), head_y - int(head_r*0.3)),
        (cx + int(head_r*1.1), head_y - int(head_r*0.3)),
        (cx + int(head_r*0.3), head_y - int(head_r*2.2))
    ]
    draw.polygon(hat_pts, fill=hat_purple, outline=OUTLINE)
    # 帽尖弯曲
    tip_x = cx + int(head_r*0.3)
    tip_y = head_y - int(head_r*2.2)
    curve_end = (tip_x + int(12*s), tip_y + int(5*s))
    draw.line([tip_x, tip_y, curve_end[0], curve_end[1]], fill=hat_purple, width=int(6*s))
    # 铃铛
    draw_ellipse_outline(draw,
        [curve_end[0] - int(4*s), curve_end[1] - int(4*s),
         curve_end[0] + int(4*s), curve_end[1] + int(4*s)],
        hat_star, width=1)
    # 星星
    for i in range(3):
        sx = cx + (i * 10 - 10) * int(s)
        sy = head_y - int(head_r*1.0) - i * int(5*s)
        draw.ellipse([sx - int(2.5*s), sy - int(2.5*s), sx + int(2.5*s), sy + int(2.5*s)],
                     fill=hat_star)

    # 眼睛
    for side in [-1, 1]:
        ex = cx + int(side * 9 * s)
        ey = head_y + int(2*s)
        draw_ellipse_outline(draw,
            [ex - int(6.5*s), ey - int(6.5*s), ex + int(6.5*s), ey + int(6.5*s)],
            (255, 255, 255), width=1)
        draw.ellipse([ex + int(1.5*s) - int(3.5*s), ey - int(3.5*s),
                      ex + int(1.5*s) + int(3.5*s), ey + int(3.5*s)],
                     fill=(204, 51, 128))
        draw.ellipse([ex + int(2.5*s) - int(1.5*s), ey - int(1*s) - int(1.5*s),
                      ex + int(2.5*s) + int(1.5*s), ey - int(1*s) + int(1.5*s)],
                     fill=(255, 255, 255))
        # 弯眉
        draw.arc([ex - int(6*s), ey - int(14*s), ex + int(6*s), ey - int(2*s)],
                 36, 144, fill=OUTLINE, width=int(2.5*s*0.3))

    # 嘴巴
    draw.arc([cx - int(5*s), head_y + int(10*s) - int(5*s),
              cx + int(5*s), head_y + int(10*s) + int(5*s)],
             6, 174, fill=OUTLINE, width=int(2*s*0.3))
    # 腮红
    draw.ellipse([cx - int(14*s) - int(4*s), head_y + int(6*s) - int(4*s),
                  cx - int(14*s) + int(4*s), head_y + int(6*s) + int(4*s)],
                 fill=(255, 153, 153, 76))
    draw.ellipse([cx + int(14*s) - int(4*s), head_y + int(6*s) - int(4*s),
                  cx + int(14*s) + int(4*s), head_y + int(6*s) + int(4*s)],
                 fill=(255, 153, 153, 76))


def head_y_pos(draw, body_top, head_r):
    return body_top - head_r


# ════════════════════════════════════════════════════════════
# 角色3: 萌鬼狂战士 · 小鬼丸
# ════════════════════════════════════════════════════════════
def draw_kiguemaru(draw, cx, cy, s):
    skin = (179, 157, 219)
    skin_dark = (126, 86, 194)
    belly_red = (198, 40, 40)
    armor = (66, 66, 66)
    guard = (33, 33, 33)
    horn = (255, 23, 68)
    hammer_candy = (255, 102, 153)
    hammer_brown = (93, 64, 55)
    shorts = (66, 66, 66)
    bell = (255, 215, 0)

    head_r = int(27 * s)
    body_w = int(40 * s)   # 宽
    body_h = int(38 * s)
    leg_w = int(14 * s)
    leg_h = int(22 * s)
    arm_w = int(12 * s)
    arm_h = int(28 * s)

    body_top = cy - body_h

    # 腿
    for side in [-1, 1]:
        lx = cx + int(side * 12 * s)
        # 短裤
        draw_rect_outline(draw,
            [lx - leg_w//2, body_top + body_h, lx + leg_w//2, body_top + body_h + int(leg_h*0.5)],
            shorts, width=2)
        # 紫色小腿
        draw.rectangle([lx - leg_w//2, body_top + body_h + int(leg_h*0.5),
                        lx + leg_w//2, body_top + body_h + leg_h], fill=skin)
        # 厚脚
        draw_rect_outline(draw,
            [lx - int(leg_w*0.7), body_top + body_h + leg_h,
             lx + int(leg_w*0.7), body_top + body_h + leg_h + int(6*s)],
            skin_dark, width=2)

    # 身体（肚兜）
    draw_rect_outline(draw,
        [cx - body_w//2, body_top, cx + body_w//2, body_top + body_h],
        belly_red, width=3)
    # 带子
    draw.line([cx - body_w//2, body_top + int(5*s), cx + body_w//2, body_top + int(5*s)],
              fill=(128, 26, 26), width=int(3*s))
    draw.line([cx, body_top, cx, body_top + body_h],
              fill=(128, 26, 26), width=int(3*s))
    # "鬼"字
    draw.ellipse([cx - int(6*s), body_top + int(body_h*0.35) - int(6*s),
                  cx + int(6*s), body_top + int(body_h*0.35) + int(6*s)],
                 fill=(51, 13, 13))
    draw.ellipse([cx - int(2*s), body_top + int(body_h*0.35) - int(3*s) - int(2*s),
                  cx + int(2*s), body_top + int(body_h*0.35) - int(3*s) + int(2*s)],
                 fill=(51, 13, 13))
    # 粗绳腰带
    draw_rect_outline(draw,
        [cx - int(body_w*0.6), body_top + int(body_h*0.85),
         cx + int(body_w*0.6), body_top + int(body_h*0.85) + int(6*s)],
        (161, 136, 127), width=1)

    # 肩甲（不对称）
    # 左大
    lsx = cx - body_w//2 - int(5*s)
    draw_rect_outline(draw,
        [lsx - int(12*s), body_top - int(5*s), lsx + int(12*s), body_top + int(13*s)],
        armor, width=2)
    draw.rectangle([lsx - int(10*s), body_top - int(3*s),
                    lsx + int(10*s), body_top], fill=(89, 89, 89))
    # 右小
    rsx = cx + body_w//2 + int(3*s)
    draw_rect_outline(draw,
        [rsx - int(8*s), body_top, rsx + int(8*s), body_top + int(12*s)],
        armor, width=2)

    # 手臂
    for side in [-1, 1]:
        ax = cx + int(side * (body_w//2 + arm_w//2 + 4*s))
        # 紫色上臂
        draw.rectangle([ax - arm_w//2, body_top + int(8*s),
                        ax + arm_w//2, body_top + int(8*s) + int(arm_h*0.5)], fill=skin)
        # 黑铁护臂
        draw_rect_outline(draw,
            [ax - arm_w//2 - int(1*s), body_top + int(8*s) + int(arm_h*0.5),
             ax + arm_w//2 + int(1*s), body_top + int(8*s) + arm_h],
            guard, width=2)
        # 拳头
        fist_y = body_top + int(8*s) + arm_h + int(2*s)
        draw_ellipse_outline(draw,
            [ax - int(7*s), fist_y - int(7*s), ax + int(7*s), fist_y + int(7*s)],
            skin, width=2)

    # 巨锤
    hammer_cx_pos = cx + int((body_w//2 + arm_w + 25*s))
    hammer_head_w = int(50 * s)
    hammer_head_h = int(40 * s)
    hammer_top = body_top - int(35 * s)
    # 柄
    draw_rect_outline(draw,
        [hammer_cx_pos - int(3*s), hammer_top, hammer_cx_pos + int(3*s), body_top + body_h + leg_h],
        hammer_brown, width=2)
    # 锤头
    hh_x = hammer_cx_pos - hammer_head_w//2
    hh_y = hammer_top - hammer_head_h//2
    draw_rect_outline(draw,
        [hh_x, hh_y, hh_x + hammer_head_w, hh_y + hammer_head_h],
        hammer_candy, width=3)
    # 条纹
    stripe_colors = [(255, 255, 102), (102, 204, 255), (128, 255, 179), (255, 153, 102)]
    for i in range(4):
        sy = hh_y + i * hammer_head_h // 4
        draw.rectangle([hh_x, sy, hh_x + hammer_head_w, sy + hammer_head_h // 8],
                       fill=stripe_colors[i])
    # 金属光泽
    draw.rectangle([hh_x + int(2*s), hh_y + int(2*s),
                    hh_x + hammer_head_w - int(2*s), hh_y + int(3*s)],
                   fill=(255, 255, 255, 51))

    # 铃铛
    draw_ellipse_outline(draw,
        [cx - int(4*s), body_top - int(3*s) - int(4*s),
         cx + int(4*s), body_top - int(3*s) + int(4*s)],
        bell, width=1)

    # 头部
    head_y = body_top - head_r
    draw_ellipse_outline(draw,
        [cx - head_r, head_y - head_r, cx + head_r, head_y + head_r],
        skin, width=3)

    # 鬼角
    for side in [-1, 1]:
        hx = cx + int(side * 12 * s)
        horn_base = head_y - int(head_r * 0.7)
        horn_tip = head_y - int(head_r * 1.4)
        horn_pts = [
            (hx - int(5*s), horn_base),
            (hx, horn_tip),
            (hx + int(5*s), horn_base)
        ]
        draw.polygon(horn_pts, fill=horn, outline=OUTLINE)
        # 角发光
        draw.ellipse([hx - int(4*s), horn_tip - int(4*s),
                      hx + int(4*s), horn_tip + int(4*s)],
                     fill=(255, 109, 0, 76))

    # 尖耳朵
    for side in [-1, 1]:
        ex = cx + int(side * head_r * 0.9)
        ear_pts = [
            (ex, head_y),
            (ex + int(side * 10*s), head_y - int(5*s)),
            (ex + int(side * 4*s), head_y + int(5*s))
        ]
        draw.polygon(ear_pts, fill=skin, outline=OUTLINE)

    # 眼睛
    for side in [-1, 1]:
        ex = cx + int(side * 10 * s)
        ey = head_y + int(1*s)
        draw_ellipse_outline(draw,
            [ex - int(7*s), ey - int(7*s), ex + int(7*s), ey + int(7*s)],
            (255, 255, 230), width=1)
        draw.ellipse([ex + int(1.5*s) - int(4*s), ey - int(4*s),
                      ex + int(1.5*s) + int(4*s), ey + int(4*s)],
                     fill=(255, 215, 0))
        draw.ellipse([ex + int(2*s) - int(2*s), ey - int(2*s),
                      ex + int(2*s) + int(2*s), ey + int(2*s)],
                     fill=(26, 26, 26))
        draw.ellipse([ex + int(3*s) - int(1.5*s), ey - int(1.5*s) - int(1.5*s),
                      ex + int(3*s) + int(1.5*s), ey - int(1.5*s) + int(1.5*s)],
                     fill=(255, 255, 255))
        draw.line([ex - int(7*s), ey - int(9*s),
                   ex + int(7*s), ey - int(8*s)], fill=OUTLINE, width=int(3.5*s*0.3))

    # 锯齿嘴
    mouth_y = head_y + int(12*s)
    for i in range(4):
        mx = cx + (i * 5 - 7) * int(s)
        draw.line([mx, mouth_y, mx + int(2.5*s), mouth_y + int(3*s)], fill=OUTLINE, width=int(2.5*s*0.3))
        draw.line([mx + int(2.5*s), mouth_y + int(3*s), mx + int(5*s), mouth_y], fill=OUTLINE, width=int(2.5*s*0.3))


# ════════════════════════════════════════════════════════════
# 主渲染
# ════════════════════════════════════════════════════════════
def render():
    img = Image.new('RGBA', (W, H), (*BG_DARK, 255))
    draw = ImageDraw.Draw(img)

    # 背景渐变
    for y in range(80):
        alpha = int((80 - y) / 80 * 76)
        draw.line([(0, y), (W, y)], fill=(0, 0, 0, alpha))

    # 地面
    floor_y = CHAR_Y + 10
    draw.rectangle([0, floor_y, W, H], fill=(*BG_FLOOR, 255))
    draw.line([(0, floor_y), (W, floor_y)], fill=(102, 102, 128), width=2)
    # 石砖
    for x in range(0, W, 64):
        draw.line([(x, floor_y), (x, H)], fill=(38, 38, 56), width=1)
    for yo in [32, 64, 96]:
        draw.line([(0, floor_y + yo), (W, floor_y + yo)], fill=(38, 38, 56), width=1)

    # 氛围粒子
    for i in range(20):
        px = (i * 137 + 50) % W
        py = (i * 89) % int(H * 0.7)
        sz = 1 + (i * 3) % 2
        al = 20 + (i * 7) % 20
        draw.ellipse([px - sz, py - sz, px + sz, py + sz], fill=(255, 255, 255, al))

    # 标题
    draw_text_cn(draw, "格斗萌主 — 角色视觉定稿 v1.0", W//2, 15, size=24, fill=(240, 240, 240))
    draw_text_cn(draw, "Q版2D横版格斗 | 三头身比例 | 糖果炸裂风", W//2, 45, size=14, fill=(179, 179, 204))

    # 角色
    s = SCALE
    draw_huikong(draw, CHAR_1_X, CHAR_Y, s)
    draw_tangtang(draw, CHAR_2_X, CHAR_Y, s)
    draw_kiguemaru(draw, CHAR_3_X, CHAR_Y, s)

    # 标签框
    labels = [
        (CHAR_1_X, "[武僧] 气功小武僧", "慧空 · Blazing Monkfist", "近战连招 | 气功层数", (51, 102, 204)),
        (CHAR_2_X, "[魔导] 糖果魔导师", "糖糖 · Sugar Hexblade", "中程控场 | 糖果配方", (255, 107, 157)),
        (CHAR_3_X, "[狂战] 萌鬼狂战士", "小鬼丸 · Oni Crushkin", "近战爆发 | 怒气变身", (155, 89, 182)),
    ]

    for lx, title, name, role, accent in labels:
        box_w, box_h = 220, 75
        bx = lx - box_w // 2
        by = CHAR_Y + 200
        # 背景
        draw.rectangle([bx, by, bx + box_w, by + box_h], fill=(26, 26, 38, 179))
        # 色条
        draw.rectangle([bx, by, bx + box_w, by + 4], fill=accent)
        # 文字
        draw_text_cn(draw, title, lx, by + 12, size=16, fill=(240, 240, 240))
        draw_text(draw, name, lx, by + 34, size=13, fill=accent)
        draw_text_cn(draw, role, lx, by + 54, size=11, fill=(153, 153, 179))

    # 分隔线
    for lx in [CHAR_1_X + 150, CHAR_2_X - 150, CHAR_2_X + 150, CHAR_3_X - 150]:
        draw.line([(lx, CHAR_Y - 150), (lx, CHAR_Y + 180)], fill=(60, 60, 80, 100), width=1)

    # 底部说明
    draw_text_cn(draw, "辨识度: 主色唯一 | 剪影唯一 | 32px可辨 | 特效色唯一 | 轮廓形状(圆/三角/方)",
                 W//2, H - 25, size=11, fill=(128, 128, 153))

    # 保存
    out = "/tmp/first-game-characters/characters_final.png"
    os.makedirs(os.path.dirname(out), exist_ok=True)
    img.convert('RGB').save(out, 'PNG', quality=95)
    print(f"✅ 已保存: {out} ({os.path.getsize(out)} bytes)")

    # 也保存一个无alpha的版本
    out2 = "/tmp/first-game-characters/characters_final_noalpha.png"
    img_rgb = Image.new('RGB', (W, H), BG_DARK)
    img_rgb.paste(img, mask=img.split()[3] if img.mode == 'RGBA' else None)
    img_rgb.save(out2, 'PNG', quality=95)
    print(f"✅ 已保存: {out2}")

    return out

if __name__ == '__main__':
    render()

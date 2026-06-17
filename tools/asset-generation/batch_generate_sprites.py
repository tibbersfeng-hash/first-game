#!/usr/bin/env python3
"""批量生成角色 sprite 关键帧 — 使用 wanx2.1-t2i-plus (高质量)"""

import os, time, urllib.request
from dashscope import ImageSynthesis
import dashscope

dashscope.api_key = os.environ.get("DASHSCOPE_API_KEY", "")
if not dashscope.api_key:
    raise RuntimeError("Set DASHSCOPE_API_KEY env var before running this script")

OUT = "/home/claude/.frontend/first-game/design/narrative/characters/artwork/sprites"

# ── 角色 + 关键动作 prompt ──────────────────────────────────────
JOBS = {
    "huikong": {
        "base": "超Q版二头身chibi中国武僧小男孩，短发小发，金色额饰，红色头带飘带，深蓝交领武僧袍云纹，金色袈裟披左肩，红腰带蝴蝶结，大颗棕色念珠，金色护手拳套，白绑腿灰布鞋",
        "actions": {
            "idle":     (4, "站立待机呼吸微动，自然放松，正面朝向，动画关键帧", "wanx2.1-t2i-plus"),
            "light1":   (3, "快速右直拳攻击，白色气功弧特效，动态出拳姿势", "wanx2.1-t2i-turbo"),
            "light3":   (3, "双拳升龙上勾攻击，金色龙形气功波特效，击飞姿势", "wanx2.1-t2i-turbo"),
            "heavy":    (3, "双掌前推重攻，金色圆形冲击波特效，蓄力释放姿势", "wanx2.1-t2i-turbo"),
            "special":  (3, "必杀如来神掌，全身金光蓄力，巨型金色佛掌从天而降", "wanx2.1-t2i-plus"),
            "hit":      (2, "被击中受击硬直，身体挤压变形，痛苦表情，白色星星粒子", "wanx2.1-t2i-turbo"),
        }
    },
    "tangtang": {
        "base": "超Q版二头身chibi中国仙侠小魔女，双环望仙髻金色发簪，粉色长发刘海，额头红花钿，粉色齐胸襦裙薄荷绿边，薄荷绿披帛水袖，手持葫芦法杖飘丹气，翘头绣鞋",
        "actions": {
            "idle":     (4, "站立待机，裙摆轻摆披帛飘动，葫芦法杖冒彩色丹气，甜美微笑，正面朝向", "wanx2.1-t2i-plus"),
            "light1":   (3, "法杖挥出粉色糖果弹丸攻击，弹丸带尾迹特效", "wanx2.1-t2i-turbo"),
            "light3":   (3, "法杖释放巨大糖果炮弹攻击，蓄力后爆炸特效", "wanx2.1-t2i-turbo"),
            "heavy":    (3, "蹲下法杖点地放置蓝色糖果地雷，魔法阵展开发光", "wanx2.1-t2i-turbo"),
            "special":  (3, "必杀糖果风暴，跳起法杖指天，彩虹魔法阵展开糖果如雨倾泻", "wanx2.1-t2i-plus"),
            "hit":      (2, "被击中受击，表情从甜美变暴怒，身体挤压变形", "wanx2.1-t2i-turbo"),
        }
    },
    "kiguemaru": {
        "base": "超Q版二头身chibi中国山海经小妖怪，圆滚滚大头浅紫皮肤，朱红弯角，金色大眼两颗獠牙锯齿嘴，金色铃铛骷髅挂饰，朱红肚兜云纹鬼字，兽面肩甲，铁指虎，粗绳腰带，小恶魔尾巴心形末端",
        "actions": {
            "idle":     (4, "站立待机，狼牙棒扛肩上，身体左右晃动，角闪微光，憨厚表情，正面朝向", "wanx2.1-t2i-plus"),
            "light1":   (3, "单手横扫狼牙棒攻击，暗紫色弧形冲击波特效", "wanx2.1-t2i-turbo"),
            "light3":   (3, "双手过顶狼牙棒砸地攻击，地面裂缝暗紫震荡波AOE特效", "wanx2.1-t2i-turbo"),
            "heavy":    (3, "跳起后狼牙棒重砸地面，大范围圆形冲击波特效", "wanx2.1-t2i-turbo"),
            "special":  (3, "必杀鬼王降临，全身鬼化暗紫气焰血红眼，巨锤连续砸地地裂", "wanx2.1-t2i-plus"),
            "hit":      (2, "被击中快速弹回，凶狠表情，紫色火花粒子", "wanx2.1-t2i-turbo"),
        }
    },
}


def generate(char_key, char_cfg, action_name, frame_num, total, action_desc, model):
    char_name = {"huikong": "慧空", "tangtang": "糖糖", "kiguemaru": "小鬼丸"}[char_key]
    prompt = (
        f"{char_cfg['base']}，动作：{action_desc}（动画序列第{frame_num}帧/共{total}帧），"
        f"2D手绘赛璐璐风格，干净黑色描边线条，柔和渐变上色，白色背景，"
        f"游戏角色精灵帧动画，高质量"
    )
    out_dir = f"{OUT}/{char_key}"
    os.makedirs(out_dir, exist_ok=True)
    out_path = f"{out_dir}/{action_name}_{frame_num:02d}.png"

    # 跳过已存在的
    if os.path.exists(out_path) and os.path.getsize(out_path) > 10000:
        print(f"    ⏭ 已存在")
        return out_path

    rsp = ImageSynthesis.call(
        model=model, prompt=prompt, n=1, size="1024*1024",
        seed=hash(f"{char_key}_{action_name}_{frame_num}_{model}") % (2**31),
    )

    if rsp.status_code == 200 and rsp.output and rsp.output.results:
        url = rsp.output.results[0].url
        urllib.request.urlretrieve(url, out_path)
        size = os.path.getsize(out_path)
        return out_path
    else:
        print(f"    ❌ {getattr(rsp, 'message', '?')}")
        return None


def main():
    total = 0
    success = 0
    for char_key, char_cfg in JOBS.items():
        name = {"huikong": "慧空", "tangtang": "糖糖", "kiguemaru": "小鬼丸"}[char_key]
        print(f"\n{'━'*50}")
        print(f"  {name} ({char_key})")
        print(f"{'━'*50}")

        for action, (frames, desc, model) in char_cfg["actions"].items():
            print(f"\n   {action} ({frames}帧, {model}) — {desc[:20]}...")
            for i in range(1, frames + 1):
                total += 1
                print(f"    帧 {i}/{frames} [{model.split('-')[-1]}]...", end=" ", flush=True)
                result = generate(char_key, char_cfg, action, i, frames, desc, model)
                if result:
                    print(f"✅ {os.path.getsize(result)//1024}KB")
                    success += 1
                else:
                    print("❌")
                time.sleep(2)

    print(f"\n\n{'━'*50}")
    print(f"  完成: {success}/{total} 帧成功")
    print(f"  目录: {OUT}/")
    print(f"{'━'*50}")


if __name__ == "__main__":
    main()

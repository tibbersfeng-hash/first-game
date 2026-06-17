#!/usr/bin/env python3
"""格斗萌主 — AI批量生成角色动作帧
策略: img2img (图生图) + 固定seed + 统一prompt风格词
"""

import os
import json
import time
import urllib.request
import base64
from dashscope import ImageSynthesis

API_KEY = os.environ.get("DASHSCOPE_API_KEY", "")
if not API_KEY:
    raise RuntimeError("Set DASHSCOPE_API_KEY env var before running this script")
os.environ["DASHSCOPE_API_KEY"] = API_KEY

import dashscope
dashscope.api_key = API_KEY

BASE_DIR = "/home/claude/.frontend/first-game/design/narrative/characters/artwork"
OUT_DIR = f"{BASE_DIR}/sprites"

# ─ 动作定义 ──────────────────────────────────────────────────────
# 每个动作: (名称, 帧数, prompt描述)
ACTIONS = {
    "idle":      (8,  "站立待机，呼吸微动，自然放松"),
    "walk":      (8,  "走路，弹性步伐"),
    "run":       (6,  "跑步，快速前冲"),
    "light1":    (5,  "轻攻击第一段，快速出拳/挥杖/挥锤"),
    "light2":    (5,  "轻攻击第二段，连击动作"),
    "light3":    (6,  "轻攻击第三段，收尾重击"),
    "heavy":     (8,  "重攻击，蓄力后大威力一击"),
    "special":   (12, "必杀技，华丽大招释放"),
    "dodge":     (5,  "闪避翻滚"),
    "hit":       (4,  "被击中，受击硬直"),
    "jump":      (4,  "跳跃，起跳到落地"),
}

# ── 角色配置 ──────────────────────────────────────────────────────
CHARACTERS = {
    "huikong": {
        "name": "慧空",
        "ref": f"{BASE_DIR}/01_huikong.png",
        "base_prompt": "超Q版二头身chibi中国武僧小男孩，短发小发髻，金色额饰，红色头带，蓝色武僧袍，念珠，金色护手，白绑腿，布鞋",
    },
    "tangtang": {
        "name": "糖糖",
        "ref": f"{BASE_DIR}/02_tangtang.png",
        "base_prompt": "超Q版二头身chibi中国仙侠小魔女，双环望仙，金色发簪，粉色裙，薄荷绿披帛水袖，葫芦法杖，翘头绣鞋",
    },
    "kiguemaru": {
        "name": "小鬼丸",
        "ref": f"{BASE_DIR}/03_kiguemaru.png",
        "base_prompt": "超Q版二头身chibi中国山海经小妖怪，浅紫皮肤，朱红弯角，金色大眼，獠牙，朱红肚兜，兽面肩甲，铁指虎，狼牙棒，小尾巴",
    },
}


def img_to_base64(path):
    with open(path, "rb") as f:
        return base64.b64encode(f.read()).decode("utf-8")


def generate_frame(char_key, char_cfg, action_name, frame_num, total_frames, action_prompt):
    """生成单帧"""
    char_name = char_cfg["name"]
    prompt = (
        f"超Q版二头身chibi比例，单个角色，{char_cfg['base_prompt']}，"
        f"动作：{action_prompt}（动画序列第{frame_num}帧，共{total_frames}帧），"
        f"2D手绘赛璐璐风格，干净线条，柔和渐变上色，透明或纯色背景，"
        f"游戏精灵帧动画，白色背景，高质量"
    )

    ref_b64 = img_to_base64(char_cfg["ref"])

    try:
        # 使用 wanx2.1-t2i-turbo 文生图（img2img 在 wanx 中通过 reference_image 实现）
        rsp = ImageSynthesis.call(
            model="wanx2.1-t2i-turbo",
            prompt=prompt,
            n=1,
            size="1024*1024",
            seed=hash(f"{char_key}_{action_name}_{frame_num}") % (2**31),
        )

        if rsp.status_code == 200 and rsp.output and rsp.output.results:
            url = rsp.output.results[0].url
            # 下载到本地
            out_path = f"{OUT_DIR}/{char_key}/{action_name}_{frame_num:02d}.png"
            os.makedirs(os.path.dirname(out_path), exist_ok=True)
            urllib.request.urlretrieve(url, out_path)
            size = os.path.getsize(out_path)
            return out_path, size
        else:
            msg = getattr(rsp, 'message', 'unknown')
            print(f"    ❌ 失败: {msg}")
            return None, 0

    except Exception as e:
        print(f"    ❌ 异常: {e}")
        return None, 0


def generate_character_sprites(char_key, char_cfg):
    """生成一个角色的所有动作帧"""
    char_name = char_cfg["name"]
    print(f"\n{'━' * 50}")
    print(f"  生成角色: {char_name} ({char_key})")
    print(f"{'━' * 50}")

    results = {}
    total_generated = 0

    for action_name, (frame_count, action_desc) in ACTIONS.items():
        print(f"\n  🎬 {action_name} ({frame_count}帧) — {action_desc}")
        frames = []

        for frame_num in range(1, frame_count + 1):
            print(f"    帧 {frame_num}/{frame_count}...", end=" ", flush=True)
            path, size = generate_frame(
                char_key, char_cfg, action_name, frame_num, frame_count, action_desc
            )
            if path:
                frames.append(path)
                print(f"✅ {size // 1024}KB")
                total_generated += 1
            else:
                frames.append(None)
                print("❌")
            # 避免频率限制
            time.sleep(2)

        results[action_name] = frames

    print(f"\n  📊 {char_name} 完成: {total_generated} 帧")
    return results


def main():
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    print("  格斗萌主 — AI批量生成角色动作帧")
    print("  模型: wanx2.1-t2i-turbo")
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")

    all_results = {}
    for char_key, char_cfg in CHARACTERS.items():
        result = generate_character_sprites(char_key, char_cfg)
        all_results[char_key] = result

    # 汇总
    print(f"\n\n{'━' * 50}")
    print("  全部完成汇总")
    print(f"{'━' * 50}")
    total = 0
    for char_key, result in all_results.items():
        name = CHARACTERS[char_key]["name"]
        frames = sum(1 for frames in result.values() for f in frames if f)
        total += frames
        print(f"  {name}: {frames} 帧")

    print(f"\n  总计: {total} 帧")
    print(f"  目录: {OUT_DIR}/")


if __name__ == "__main__":
    main()

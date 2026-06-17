#!/usr/bin/env python3
"""优化批量生成 — 解决帧间一致性、hit帧风格、背景残留问题"""

import os, time, urllib.request, base64
from dashscope import ImageSynthesis
import dashscope

dashscope.api_key = os.environ.get("DASHSCOPE_API_KEY", "")
if not dashscope.api_key:
    raise RuntimeError("Set DASHSCOPE_API_KEY env var before running this script")

BASE = "/home/claude/.frontend/first-game/design/narrative/characters/artwork/sprites"
OUT = f"{BASE}/optimized"

# ── 通用负面提示词（解决背景残留+3D风格混入）──
NEGATIVE = "3D渲染, 写实风格, 照片质感, 复杂背景, 地面阴影, 灰色背景, 多余元素, 低质量, 模糊, 变形, 多余手指"

# ── 用每个角色最好的帧作为参考图 ─
REF_IMAGES = {
    "huikong": f"{BASE}/huikong/heavy_01.png",    # 金色气功拳，风格最好
    "tangtang": f"{BASE}/tangtang/light1_01.png",  # 糖果弹攻击，最可爱
    "kiguemaru": f"{BASE}/kiguemaru/light3_02.png", # 砸地攻击，最凶萌
}

# ── 优化任务 ──
JOBS = {
    "huikong": {
        "base": "超Q版二头身chibi中国武僧小男孩，短发小发髻，红色头带飘带，深蓝交领武僧袍云纹，金色袈裟披左肩，红腰带蝴蝶结，大颗棕色念珠，金色护手拳套，白绑腿灰布鞋",
        "actions": {
            "hit": (3, "被击中受击硬直，身体向后仰，痛苦咬牙表情，白色星星粒子", "wanx2.1-t2i-turbo"),
            "idle": (4, "站立待机呼吸微动，自然放松正面朝向，金色护手抱于身前", "wanx2.1-t2i-plus"),
            "light1": (3, "快速右直拳攻击，白色小型气功弧特效，动态出拳姿势身体前倾", "wanx2.1-t2i-turbo"),
        }
    },
    "tangtang": {
        "base": "超Q版二头身chibi中国仙侠小魔女，双环望仙金色发簪，粉色长发刘海额头红花钿，粉色齐胸襦裙薄荷绿边，薄荷绿半透明披帛水袖，手持竹节法杖顶端红色葫芦丹炉飘彩色丹气，粉色翘头绣花鞋",
        "actions": {
            "hit": (3, "被击中受击，表情从甜美变暴怒，身体向后仰，粉色魔法粒子飞散", "wanx2.1-t2i-turbo"),
            "idle": (4, "站立待机裙摆轻摆披帛飘动，葫芦法杖冒彩色丹气，甜美微笑正面朝向", "wanx2.1-t2i-plus"),
        }
    },
    "kiguemaru": {
        "base": "超Q版二头身chibi中国山海经小妖怪，圆滚滚大头浅紫皮肤，朱红向后弯角金色大眼两颗白色獠牙锯齿嘴，金色小铃铛骷髅挂饰，朱红肚兜云纹鬼字纹章，不对称铁灰兽面肩甲，黑色铁护臂铁指虎，深灰短裤粗麻绳腰带，小恶魔尾巴红色心形末端",
        "actions": {
            "hit": (3, "被击中快速弹回，凶狠咬牙表情，紫色火花粒子，身体微压缩", "wanx2.1-t2i-turbo"),
            "idle": (4, "站立待机狼牙棒扛肩上身体微晃，角闪橙光铃铛轻响，憨厚表情正面朝向", "wanx2.1-t2i-plus"),
        }
    },
}


def load_ref_b64(path):
    if os.path.exists(path):
        with open(path, "rb") as f:
            return base64.b64encode(f.read()).decode()
    return None


def generate_optimized(char_key, char_cfg, action_name, frame_num, total, action_desc, model, ref_b64):
    prompt = (
        f"{char_cfg['base']}，动作：{action_desc}（动画序列第{frame_num}帧/共{total}帧），"
        f"2D手绘赛璐璐风格，干净粗黑描边线条，柔和渐变平涂上色，"
        f"纯白色背景无任何阴影，游戏角色精灵帧动画，高质量"
    )

    out_dir = f"{OUT}/{char_key}"
    os.makedirs(out_dir, exist_ok=True)
    out_path = f"{out_dir}/{action_name}_{frame_num:02d}.png"

    if os.path.exists(out_path) and os.path.getsize(out_path) > 10000:
        print(f"    ⏭ skip")
        return out_path

    kwargs = dict(
        model=model, prompt=prompt, n=1, size="1024*1024",
        negative_prompt=NEGATIVE,
        seed=hash(f"opt_{char_key}_{action_name}_{frame_num}") % (2**31),
    )

    # 如果有参考图，使用 img2img
    if ref_b64:
        kwargs["reference_images"] = [{"image": f"data:image/png;base64,{ref_b64}"}]

    rsp = ImageSynthesis.call(**kwargs)

    if rsp.status_code == 200 and rsp.output and rsp.output.results:
        url = rsp.output.results[0].url
        urllib.request.urlretrieve(url, out_path)
        return out_path
    else:
        print(f"    ❌ {getattr(rsp, 'message', '?')}")
        return None


def main():
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    print("  优化批量生成 — 一致性+风格+背景修复")
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    print(f"负面提示: {NEGATIVE[:60]}...")

    total = 0
    success = 0

    for char_key, char_cfg in JOBS.items():
        ref_path = REF_IMAGES.get(char_key)
        ref_b64 = load_ref_b64(ref_path) if ref_path else None
        print(f"\n参考图: {ref_path} ({'✅ loaded' if ref_b64 else '❌ missing'})")

        name_map = {"huikong": "慧空", "tangtang": "糖糖", "kiguemaru": "小鬼丸"}
        print(f"\n{'━'*50}")
        print(f"  {name_map[char_key]}")
        print(f"{'━'*50}")

        for action, (frames, desc, model) in char_cfg["actions"].items():
            print(f"\n   {action} ({frames}帧)")
            for i in range(1, frames + 1):
                total += 1
                print(f"    帧 {i}/{frames}...", end=" ", flush=True)
                result = generate_optimized(
                    char_key, char_cfg, action, i, frames, desc, model, ref_b64
                )
                if result:
                    print(f"✅ {os.path.getsize(result)//1024}KB")
                    success += 1
                else:
                    print("❌")
                time.sleep(2.5)  # 稍长间隔避免限流

    print(f"\n{'━'*50}")
    print(f"  完成: {success}/{total}")
    print(f"  目录: {OUT}/")


if __name__ == "__main__":
    main()

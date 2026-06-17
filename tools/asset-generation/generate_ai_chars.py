#!/usr/bin/env python3
"""使用 DashScope 通义万相 API 生成格斗萌主角色图"""

import os
import dashscope
from dashscope import ImageSynthesis
import json
import time

API_KEY = os.environ.get("DASHSCOPE_API_KEY", "")
if not API_KEY:
    raise RuntimeError("Set DASHSCOPE_API_KEY env var before running this script")
dashscope.api_key = API_KEY

OUTPUT_DIR = "/tmp/first-game-characters/ai_generated"
os.makedirs(OUTPUT_DIR, exist_ok=True)

# 角色 prompt
CHARACTERS = {
    "huikong": {
        "name": "气功小武僧·慧空",
        "prompt": "Q版三头身中国武僧小男孩角色设计，短发寸头带小发髻，额头有3个金色圆形额饰，红色头带带飘带从脑后飘出，深蓝色交领武僧袍配云纹暗纹，金色半透明袈裟披在左肩，红色宽腰带系蝴蝶结飘带，脖子上挂大颗棕色木质念珠，双手戴金色护手拳套，深蓝色裤子白色绑腿，灰色武僧布鞋，金色气功光环环绕全身，表情热血自信大眼睛坚毅微笑，暖肤色，金色气功特效，暗色石砖地牢背景，游戏角色立绘风格，干净线条柔和渐变上色，高质量",
    },
    "tangtang": {
        "name": "糖果魔导师·糖糖",
        "prompt": "Q版三头身中国仙侠风小魔女角色设计，双环望仙发型两个圆环形发髻在头部两侧，金色发簪横穿两髻两端有金珠，粉色长发刘海，额头有红色花钿，柳叶眉杏眼大眼带腮红樱桃小口，粉色齐胸襦裙唐风裙摆有薄荷绿边，腰间玉佩丝带，薄荷绿半透明披帛从双肩垂下，宽大水袖袖口薄荷绿边，手持竹节法杖顶端是红色葫芦丹炉飘出彩色丹气，粉色翘头绣花鞋有金线绣花，表情甜美俏皮，白皙肤色，粉色魔法光环环绕，暗色地牢背景，游戏角色立绘风格，干净线条柔和渐变",
    },
    "kiguemaru": {
        "name": "萌鬼狂战士·小鬼丸",
        "prompt": "Q版三头身中国山海经风格小妖怪角色设计，浅紫色皮肤，头顶一对粗壮朱红色向后弯曲的大牛角有纹路角尖发橙光，尖耳朵，额头怒眉金色大眼，嘴角露出两颗白色獠牙锯齿嘴，脖子上挂金色小铃铛和小骷髅挂饰，朱红色肚兜有云纹暗纹和黑色鬼字纹章，不对称铁灰色肩甲左大右小左肩甲有兽面纹和獠牙装饰，黑色铁护臂双手戴铁指虎，深灰色短裤粗麻绳腰带，身后有一条小恶魔尾巴末端是红色心形，手持巨型狼牙棒圆柱棒身金色环纹蓝色条纹周围有白色尖刺顶端金色宝珠，厚实小脚，表情凶萌憨厚，暗紫色怒气光环，暗色地牢背景，游戏角色立绘风格",
    },
}

def generate_character(key, char_info):
    """生成单个角色图"""
    print(f"\n🎨 正在生成: {char_info['name']}...")

    try:
        rsp = ImageSynthesis.call(
            model="wanx2.1-t2i-turbo",
            prompt=char_info["prompt"],
            n=1,
            size="1024*1024",
        )

        print(f"  状态: {rsp.status_code}")

        if rsp.status_code == 200 and rsp.output and rsp.output.results:
            for i, result in enumerate(rsp.output.results):
                url = result.url
                print(f"  图片URL: {url[:80]}...")

                # 下载图片
                import urllib.request
                local_path = f"{OUTPUT_DIR}/{key}_{i+1}.png"
                urllib.request.urlretrieve(url, local_path)
                size = os.path.getsize(local_path)
                print(f"  ✅ 已保存: {local_path} ({size} bytes)")
                return local_path
        else:
            print(f"  ❌ 生成失败: {rsp.message if hasattr(rsp, 'message') else 'unknown error'}")
            # 尝试打印更多错误信息
            if hasattr(rsp, 'output'):
                print(f"  Output: {rsp.output}")
            return None

    except Exception as e:
        print(f"  ❌ 异常: {e}")
        return None


def main():
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    print("🎨 格斗萌主 — AI角色图生成 (通义万相)")
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")

    results = {}
    for key, info in CHARACTERS.items():
        path = generate_character(key, info)
        results[key] = path
        # 避免频率限制
        if path:
            time.sleep(3)

    print("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    print("📊 生成结果")
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    for key, path in results.items():
        name = CHARACTERS[key]["name"]
        if path:
            print(f"  ✅ {name}: {path}")
        else:
            print(f"  ❌ {name}: 失败")

if __name__ == "__main__":
    main()

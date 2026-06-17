#!/usr/bin/env python3
"""尝试通过 coding plan 的 base_url 调用通义万相"""

import os
import json
import urllib.request
import urllib.error

API_KEY = os.environ.get("DASHSCOPE_API_KEY", "")
if not API_KEY:
    raise RuntimeError("Set DASHSCOPE_API_KEY env var before running this script")
OUTPUT_DIR = "/tmp/first-game-characters/ai_generated"
os.makedirs(OUTPUT_DIR, exist_ok=True)

# 尝试不同的 endpoint
ENDPOINTS = [
    # 标准 DashScope 万相 endpoint
    "https://dashscope.aliyuncs.com/api/v1/services/aigc/text2image/image-synthesis",
    # coding plan 代理
    "https://coding.dashscope.aliyuncs.com/api/v1/services/aigc/text2image/image-synthesis",
]

PROMPT = "Q版三头身中国武僧小男孩，短发小发髻，金色额饰，红色头带飘带，蓝色武僧袍，金色袈裟披肩，念珠，金色护手，布鞋，金色光环，热血表情，暗色地牢背景，游戏立绘"

def try_endpoint(url, headers, body):
    """尝试调用一个 endpoint"""
    print(f"  尝试: {url[:60]}...")
    req = urllib.request.Request(
        url,
        data=json.dumps(body).encode('utf-8'),
        headers=headers,
        method='POST'
    )
    try:
        with urllib.request.urlopen(req, timeout=30) as resp:
            data = json.loads(resp.read().decode())
            print(f"  ✅ 成功! Status: {resp.status}")
            print(f"  响应: {json.dumps(data, ensure_ascii=False)[:200]}")
            return data
    except urllib.error.HTTPError as e:
        resp_body = e.read().decode()
        print(f"  ❌ HTTP {e.code}: {resp_body[:200]}")
        return None
    except Exception as e:
        print(f"  ❌ 异常: {e}")
        return None

def main():
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    print(" 尝试通过不同 endpoint 调用万相")
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")

    headers = {
        "Authorization": f"Bearer {API_KEY}",
        "Content-Type": "application/json",
        "X-DashScope-Async": "enable",
    }

    body = {
        "model": "wanx2.1-t2i-turbo",
        "input": {
            "prompt": PROMPT,
        },
        "parameters": {
            "size": "1024*1024",
            "n": 1,
        }
    }

    for endpoint in ENDPOINTS:
        result = try_endpoint(endpoint, headers, body)
        if result:
            print(f"\n✅ 找到可用 endpoint: {endpoint}")
            return
        print()

    # 尝试不带 async
    print("尝试同步调用...")
    headers_sync = {
        "Authorization": f"Bearer {API_KEY}",
        "Content-Type": "application/json",
    }
    body_sync = {
        "model": "wanx-v1",
        "input": {
            "prompt": PROMPT,
        },
        "parameters": {
            "size": "1024*1024",
            "n": 1,
        }
    }
    for endpoint in ENDPOINTS:
        result = try_endpoint(endpoint, headers_sync, body_sync)
        if result:
            return

    print("\n 所有 endpoint 都不可用")
    print("\n结论: coding plan 的 API Key 仅限 Anthropic 文本接口使用")
    print("需要单独的 DashScope 万相 API Key 才能出图")

if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""
混元 3D 专业版 API 测试脚本
用途：测试文生 3D 功能，验证生成质量
"""

import os
import sys
import time
import json
import base64
import requests
from pathlib import Path

# 腾讯云 SDK
try:
    from tencentcloud.common import credential
    from tencentcloud.ai3d.v20250513 import ai3d_client, models
except ImportError:
    print("安装腾讯云 SDK...")
    os.system("pip install tencentcloud-sdk-python")
    from tencentcloud.common import credential
    from tencentcloud.ai3d.v20250513 import ai3d_client, models


class Hunyuan3DTester:
    """混元 3D API 测试类"""

    def __init__(self, secret_id, secret_key, region="ap-guangzhou"):
        """初始化客户端"""
        self.cred = credential.Credential(secret_id, secret_key)
        self.client = ai3d_client.Ai3dClient(self.cred, region)
        self.region = region

    def submit_text_to_3d(self, prompt, enable_pbr=False, face_count=100000, model_version="3.0"):
        """提交文生 3D 任务"""
        print(f"\n📝 提交任务: {prompt[:50]}...")

        req = models.SubmitHunyuanTo3DProJobRequest()
        req.Prompt = prompt
        req.Model = model_version
        req.EnablePBR = enable_pbr
        req.FaceCount = face_count
        req.GenerateType = "Normal"
        req.ResultFormat = "GLB"  # UE5 友好格式

        try:
            resp = self.client.SubmitHunyuanTo3DProJob(req)
            job_id = resp.JobId
            print(f"✅ 任务已提交，JobId: {job_id}")
            return job_id
        except Exception as e:
            print(f"❌ 提交失败: {e}")
            return None

    def submit_image_to_3d(self, image_url, prompt=None, enable_pbr=False, face_count=100000):
        """提交图生 3D 任务"""
        print(f"\n🖼️  提交图生 3D: {image_url[:50]}...")

        req = models.SubmitHunyuanTo3DProJobRequest()
        req.ImageUrl = image_url
        if prompt:
            req.Prompt = prompt
        req.EnablePBR = enable_pbr
        req.FaceCount = face_count
        req.GenerateType = "Normal"
        req.ResultFormat = "GLB"

        try:
            resp = self.client.SubmitHunyuanTo3DProJob(req)
            job_id = resp.JobId
            print(f"✅ 任务已提交，JobId: {job_id}")
            return job_id
        except Exception as e:
            print(f"❌ 提交失败: {e}")
            return None

    def query_job(self, job_id, max_wait=600, interval=15):
        """查询任务状态并等待完成"""
        print(f"\n⏳ 查询任务状态 (JobId: {job_id})...")

        req = models.QueryHunyuanTo3DProJobRequest()
        req.JobId = job_id

        start_time = time.time()

        while True:
            elapsed = time.time() - start_time

            if elapsed > max_wait:
                print(f"❌ 超时 ({max_wait}秒)")
                return None

            try:
                resp = self.client.QueryHunyuanTo3DProJob(req)
                status = resp.Status

                if status == "WAIT":
                    print(f"  ⏸️  排队中... ({int(elapsed)}秒)", end="\r")
                elif status == "RUN":
                    print(f"  ⚙️  生成中... ({int(elapsed)}秒)", end="\r")
                elif status == "DONE":
                    print(f"\n✅ 生成完成！耗时 {int(elapsed)}秒")
                    return resp
                elif status == "FAIL":
                    print(f"\n❌ 生成失败: {resp.ErrorCode} - {resp.ErrorMessage}")
                    return None
                else:
                    print(f"\n⚠️  未知状态: {status}")
                    return None

            except Exception as e:
                print(f"\n❌ 查询出错: {e}")
                return None

            time.sleep(interval)

    def download_results(self, response, output_dir="./output"):
        """下载生成的 3D 模型"""
        Path(output_dir).mkdir(parents=True, exist_ok=True)

        print(f"\n📥 下载结果到 {output_dir}/")

        downloaded = []

        for i, file3d in enumerate(response.ResultFile3Ds):
            file_type = file3d.Type
            url = file3d.Url
            preview_url = file3d.PreviewImageUrl

            # 下载 3D 模型
            filename = f"model_{i+1}.{file_type.lower()}.zip"
            filepath = Path(output_dir) / filename

            print(f"  下载 {file_type} 模型: {filename}")
            try:
                resp = requests.get(url, timeout=120)
                resp.raise_for_status()
                with open(filepath, 'wb') as f:
                    f.write(resp.content)
                print(f"    ✅ 已保存: {filepath} ({len(resp.content)/1024:.1f} KB)")
                downloaded.append(str(filepath))
            except Exception as e:
                print(f"    ❌ 下载失败: {e}")

            # 下载预览图
            if preview_url:
                preview_filename = f"preview_{i+1}.png"
                preview_filepath = Path(output_dir) / preview_filename

                print(f"  下载预览图: {preview_filename}")
                try:
                    resp = requests.get(preview_url, timeout=60)
                    resp.raise_for_status()
                    with open(preview_filepath, 'wb') as f:
                        f.write(resp.content)
                    print(f"    ✅ 已保存: {preview_filepath}")
                except Exception as e:
                    print(f"    ❌ 下载失败: {e}")

        return downloaded

    def test_text_to_3d(self, prompt, output_dir="./output/test_text"):
        """完整测试流程：文生 3D"""
        print("=" * 60)
        print("🧪 测试：文生 3D")
        print("=" * 60)

        # 1. 提交任务
        job_id = self.submit_text_to_3d(
            prompt=prompt,
            enable_pbr=True,
            face_count=100000,
            model_version="3.0"
        )

        if not job_id:
            return False

        # 2. 等待完成
        result = self.query_job(job_id)

        if not result:
            return False

        # 3. 显示积分消耗
        print(f"\n💰 积分消耗: {result.ResultCreditConsumed}")
        print(f"📊 详细: {result.ResultCreditDetails}")

        # 4. 下载结果
        downloaded = self.download_results(result, output_dir)

        print("\n" + "=" * 60)
        print(f"✅ 测试完成！下载了 {len(downloaded)} 个文件")
        print(f"📁 输出目录: {output_dir}/")
        print("=" * 60)

        return len(downloaded) > 0


def main():
    """主函数"""
    # 从环境变量获取凭证
    secret_id = os.environ.get("TENCENTCLOUD_SECRET_ID")
    secret_key = os.environ.get("TENCENTCLOUD_SECRET_KEY")

    if not secret_id or not secret_key:
        print("❌ 错误：请设置环境变量")
        print("  export TENCENTCLOUD_SECRET_ID='your-secret-id'")
        print("  export TENCENTCLOUD_SECRET_KEY='your-secret-key'")
        sys.exit(1)

    # 测试 Prompt（Q版武僧）
    test_prompt = "Q版三头身气功小武僧角色，光头，蓝色僧衣，红色腰带，圆润可爱的卡通风格，游戏角色，正面全身，白色背景"

    # 创建测试器
    tester = Hunyuan3DTester(secret_id, secret_key)

    # 运行测试
    success = tester.test_text_to_3d(test_prompt)

    if success:
        print("\n🎉 测试成功！")
        print("\n下一步：")
        print("1. 查看 output/test_text/ 目录中的模型文件")
        print("2. 用 Blender 打开 GLB 文件检查质量")
        print("3. 如果满意，可以继续批量生成其他角色")
    else:
        print("\n😞 测试失败")
        sys.exit(1)


if __name__ == "__main__":
    main()

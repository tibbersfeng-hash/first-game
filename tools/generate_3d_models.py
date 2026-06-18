#!/usr/bin/env python3
"""
3D Model Generator for 格斗萌主
Uses tccli ai3d (腾讯云混元3D) to generate 3D models from text/images.

Pipeline:
  1. Text/Image/MultiView → SubmitHunyuanTo3DProJob → 3D model (FBX/GLB)
  2. 3D model → SubmitAutoRiggingJob → rigged model with skeleton + preset motions
  3. Download results to design/assets/output/3d/

Usage:
  # 文生3D
  python3 generate_3d_models.py --character huikong --mode text

  # 图生3D
  python3 generate_3d_models.py --character huikong --mode image --image-url <url>

  # 多视角生3D
  python3 generate_3d_models.py --character huikong --mode multiview \
      --front <url> --back <url> --left <url> --right <url>

  # 仅绑骨
  python3 generate_3d_models.py --character huikong --mode rig --model-url <url>

  # 批量生成所有角色
  python3 generate_3d_models.py --all-characters

  # 干跑
  python3 generate_3d_models.py --character huikong --dry-run
"""

import argparse
import json
import os
import subprocess
import sys
import time
from pathlib import Path

# ─── Constants ────────────────────────────────────────────────────────────────

PROJECT_ROOT = Path(__file__).parent.parent  # tools/ -> first-game/
OUTPUT_DIR = PROJECT_ROOT / "design" / "assets" / "output" / "3d"
REGION = "ap-guangzhou"

# 预设动作列表 (MotionType)
PRESET_MOTIONS = {
    1: "回旋踢", 2: "左勾拳", 3: "蓄力攻击", 4: "蓄力出拳",
    5: "二连击打", 6: "二连击打-2", 7: "后撤", 8: "受击",
    9: "受击-2", 10: "受击-3", 11: "受击倒地-1", 12: "受击倒地-2",
    13: "落地", 14: "沮丧", 15: "割喉", 16: "刺拳",
    17: "连续击打", 18: "踢腿", 19: "侧踢", 20: "打太极",
    21: "后空翻", 22: "蹲姿转体", 23: "走路-1", 24: "走路-2",
    25: "走路-3", 26: "待机-1", 27: "待机-2", 28: "街舞",
    29: "扭扭舞", 30: "左转弯", 31: "右转弯", 32: "慢跑",
    33: "慢跑-2", 34: "奔跑", 35: "冲刺跑-1", 36: "冲刺跑-2",
    37: "冲刺跑-3", 38: "原地跳-1", 39: "滑铲", 40: "向前大跳",
    41: "向前大跳-2", 42: "跨越", 43: "恐吓", 44: "向前跌倒",
    45: "右转", 46: "原地跳-2", 47: "转身", 48: "发送冲击波",
}

# ─── Character Definitions ───────────────────────────────────────────────────

CHARACTERS = {
    "huikong": {
        "name_cn": "慧空",
        "name_en": "Blazing Monkfist",
        "prompt": (
            "NOT chibi, NOT cartoon proportions, NOT super deformed, NOT big head, "
            "realistic anime game character proportions like Genshin Impact and Honkai Impact 3rd, "
            "young adult male monk character with normal human proportions (7.5 heads tall, approximately 185cm), "
            "slender athletic lean build with visible but not exaggerated muscle definition, "
            "bald head, red forehead mark (bindi), thick eyebrows, determined calm expression, "
            "amber eyes, sharp jawline, wheat-colored skin, "
            "navy blue sleeveless cross-collar monk robe with golden orange trim, "
            "wide golden orange martial arts belt with two long flowing ribbons from the back, "
            "white baggy martial arts pants tucked into dark brown boots, "
            "golden bracers on forearms with hand wraps, fingerless gloves, "
            "cel-shaded anime toon style, game-ready 3D character model, "
            "full body T-pose standing, clean white background"
        ),
        "reference_image": None,
        "multi_view_images": None,
        "enable_pbr": True,
        "face_count": 150000,
        "model_version": "3.1",
        "generate_type": "Normal",
        "result_format": "FBX",
        "rig_motions": [26, 27, 23, 34, 1, 8, 13],
        # 待机-1, 待机-2, 走路-1, 奔跑, 回旋踢, 受击, 落地
    },
    "sugar": {
        "name_cn": "糖糖",
        "name_en": "Candy Hexblade",
        "prompt": (
            "Anime style young witch character, petite 6.5 heads tall, "
            "large curved pointed witch hat (bigger than head) with golden stars and bell, "
            "pink twin-tail hair with curly ends, "
            "pink magic robe with mint green trim, white apron with candy stains, "
            "candy-colored belt with small potion bottles, lavender boots, "
            "holding giant lollipop staff (candy part bigger than head), "
            "full body standing pose, clean white background"
        ),
        "reference_image": None,
        "multi_view_images": None,
        "enable_pbr": True,
        "face_count": 100000,
        "model_version": "3.0",
        "generate_type": "Normal",
        "result_format": "FBX",
        "rig_motions": [26, 27, 23, 34, 3, 8, 13],
        # 待机-1, 待机-2, 走路-1, 奔跑, 蓄力攻击, 受击, 落地
    },
    "guizhan": {
        "name_cn": "鬼斩",
        "name_en": "Oni Crushkin",
        "prompt": (
            "Anime style young oni demon character, 7 heads tall, "
            "stocky muscular build, wider shoulders than normal, "
            "light purple skin, two short red oni horns on head, "
            "red apron/bikini armor with black demon kanji, "
            "asymmetric iron shoulder pads (large left, small right), "
            "black iron arm guards, grey shorts, rope belt, "
            "gold bell necklace, carrying giant candy hammer "
            "(hammer head bigger than body width, rainbow striped candy texture), "
            "full body standing pose, clean white background"
        ),
        "reference_image": None,
        "multi_view_images": None,
        "enable_pbr": True,
        "face_count": 150000,
        "model_version": "3.0",
        "generate_type": "Normal",
        "result_format": "FBX",
        "rig_motions": [26, 27, 23, 34, 2, 8, 13],
        # 待机-1, 待机-2, 走路-1, 奔跑, 左勾拳, 受击, 落地
    },
}

# ─── Pipeline ────────────────────────────────────────────────────────────────


class Hunyuan3DPipeline:
    """通过 tccli ai3d 调用混元3D服务"""

    def __init__(self, region=REGION, dry_run=False):
        self.region = region
        self.dry_run = dry_run

    def _run_tccli(self, args: list[str]) -> dict:
        """执行 tccli 命令并返回 JSON 结果"""
        cmd = ["tccli"] + args + ["--region", self.region, "--output", "json"]

        if self.dry_run:
            print(f"  [DRY-RUN] {' '.join(cmd)}")
            # 返回 mock 数据，让下游步骤也能跑通
            if "Submit" in args[1]:
                return {"JobId": "dry-run-fake-id"}
            elif "Query" in args[1] or "Describe" in args[1]:
                return {
                    "Status": "DONE",
                    "ResultFile3Ds": [{
                        "Type": "FBX",
                        "Url": "https://dry-run-example.com/model.fbx",
                        "PreviewImageUrl": "https://dry-run-example.com/preview.png",
                    }],
                    "ResultCreditConsumed": 0.0,
                    "ResultCreditDetails": "dry-run",
                }
            return {}

        print(f"  执行: tccli {' '.join(args[:3])}...")
        try:
            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                timeout=60,
            )
            if result.returncode != 0:
                print(f"  ❌ tccli 错误: {result.stderr.strip()}")
                return {}

            return json.loads(result.stdout)
        except subprocess.TimeoutExpired:
            print("  ❌ tccli 超时 (60s)")
            return {}
        except json.JSONDecodeError:
            print(f"  ❌ JSON 解析失败: {result.stdout[:200]}")
            return {}

    # ─── 3D 生成 ────────────────────────────────────────────────────────────

    def submit_text_to_3d(self, prompt: str, **kwargs) -> str | None:
        """文生3D → 返回 JobId"""
        args = [
            "ai3d", "SubmitHunyuanTo3DProJob",
            "--cli-unfold-argument",
            "--Prompt", prompt,
            "--Model", kwargs.get("model_version", "3.0"),
            "--EnablePBR", str(kwargs.get("enable_pbr", True)).lower(),
            "--FaceCount", str(kwargs.get("face_count", 100000)),
            "--GenerateType", kwargs.get("generate_type", "Normal"),
            "--ResultFormat", kwargs.get("result_format", "FBX"),
        ]
        return self._submit_and_get_job_id(args, f"文生3D: {prompt[:40]}...")

    def submit_image_to_3d(self, image_url: str, **kwargs) -> str | None:
        """图生3D → 返回 JobId"""
        args = [
            "ai3d", "SubmitHunyuanTo3DProJob",
            "--cli-unfold-argument",
            "--ImageUrl", image_url,
            "--Model", kwargs.get("model_version", "3.0"),
            "--EnablePBR", str(kwargs.get("enable_pbr", True)).lower(),
            "--FaceCount", str(kwargs.get("face_count", 100000)),
            "--GenerateType", kwargs.get("generate_type", "Normal"),
            "--ResultFormat", kwargs.get("result_format", "FBX"),
        ]
        return self._submit_and_get_job_id(args, f"图生3D: {image_url[:50]}...")

    def submit_multiview_to_3d(
        self,
        front: str | None = None,
        back: str | None = None,
        left: str | None = None,
        right: str | None = None,
        top: str | None = None,
        bottom: str | None = None,
        **kwargs,
    ) -> str | None:
        """多视角图生3D → 返回 JobId"""
        # 构造 MultiViewImages JSON
        views = []
        view_map = {
            "front": front, "back": back, "left": left,
            "right": right, "top": top, "bottom": bottom,
        }
        # API 支持: left, right, back, top(3.1), bottom(3.1)
        # front 不在 MultiViewImages 中，需用 ImageUrl 传入
        api_view_map = {
            "back": back, "left": left, "right": right,
            "top": top, "bottom": bottom,
        }
        for view_type, url in api_view_map.items():
            if url:
                views.append({"ViewType": view_type, "ViewImageUrl": url})

        args = [
            "ai3d", "SubmitHunyuanTo3DProJob",
            "--cli-unfold-argument",
            "--Model", kwargs.get("model_version", "3.0"),
            "--EnablePBR", str(kwargs.get("enable_pbr", True)).lower(),
            "--FaceCount", str(kwargs.get("face_count", 100000)),
            "--GenerateType", kwargs.get("generate_type", "Normal"),
            "--ResultFormat", kwargs.get("result_format", "FBX"),
        ]

        # front 作为主图传入 ImageUrl
        if front:
            args += ["--ImageUrl", front]

        if views:
            args += ["--MultiViewImages", json.dumps(views)]

        return self._submit_and_get_job_id(
            args, f"多视角生3D ({len(views)} views)"
        )

    def _extract_response(self, resp: dict) -> dict:
        """兼容 tccli 两种返回格式：有/无 Response 包装"""
        if "Response" in resp:
            return resp["Response"]
        return resp

    def _submit_and_get_job_id(self, args: list[str], label: str) -> str | None:
        """提交任务并提取 JobId"""
        print(f"\n📝 {label}")
        resp = self._run_tccli(args)
        data = self._extract_response(resp)
        job_id = data.get("JobId")
        if job_id:
            print(f"  ✅ JobId: {job_id}")
        else:
            print(f"  ❌ 提交失败: {resp}")
        return job_id

    # ─── 任务轮询 ───────────────────────────────────────────────────────────

    def wait_for_3d_job(
        self, job_id: str, max_wait: int = 1800, interval: int = 15
    ) -> dict | None:
        """轮询 3D 生成任务直到完成"""
        print(f"\n⏳ 等待 3D 生成 (JobId: {job_id})...")
        return self._poll_job(
            job_id=job_id,
            query_action="QueryHunyuanTo3DProJob",
            max_wait=max_wait,
            interval=interval,
            label="3D 生成",
        )

    def wait_for_rigging_job(
        self, job_id: str, max_wait: int = 900, interval: int = 10
    ) -> dict | None:
        """轮询绑骨任务直到完成"""
        print(f"\n⏳ 等待绑骨 (JobId: {job_id})...")
        return self._poll_job(
            job_id=job_id,
            query_action="DescribeAutoRiggingJob",
            max_wait=max_wait,
            interval=interval,
            label="绑骨",
        )

    def _poll_job(
        self,
        job_id: str,
        query_action: str,
        max_wait: int,
        interval: int,
        label: str,
    ) -> dict | None:
        """通用轮询逻辑"""
        start_time = time.time()

        while True:
            elapsed = time.time() - start_time
            if elapsed > max_wait:
                print(f"\n  ❌ {label}超时 ({max_wait}秒)")
                return None

            args = [
                "ai3d", query_action,
                "--cli-unfold-argument",
                "--JobId", job_id,
            ]
            resp = self._run_tccli(args)
            data = self._extract_response(resp)
            status = data.get("Status", "")

            if status == "WAIT":
                print(f"  ⏸️  排队中... ({int(elapsed)}秒)", end="\r")
            elif status == "RUN":
                print(f"  ⚙️  生成中... ({int(elapsed)}秒)", end="\r")
            elif status == "DONE":
                print(f"\n  ✅ {label}完成！耗时 {int(elapsed)}秒")
                return data
            elif status == "FAIL":
                err_code = data.get("ErrorCode", "")
                err_msg = data.get("ErrorMessage", "")
                print(f"\n  ❌ {label}失败: {err_code} - {err_msg}")
                return None
            else:
                print(f"\n  ⚠️  未知状态: {status}")
                return None

            time.sleep(interval)

    # ─── 自动绑骨 ───────────────────────────────────────────────────────────

    def submit_auto_rigging(
        self, model_url: str, file_type: str = "FBX", motion_type: int | None = None
    ) -> str | None:
        """提交自动绑骨任务 → 返回 JobId"""
        args = [
            "ai3d", "SubmitAutoRiggingJob",
            "--cli-unfold-argument",
            "--File3D.Url", model_url,
            "--File3D.Type", file_type,
        ]
        if motion_type is not None:
            args += ["--MotionType", str(motion_type)]

        motion_label = PRESET_MOTIONS.get(motion_type, str(motion_type)) if motion_type else "无"
        return self._submit_and_get_job_id(
            args, f"绑骨 (动作: {motion_label})"
        )

    # ─── 下载 ───────────────────────────────────────────────────────────────

    def download_results(self, response: dict, output_dir: Path) -> list[str]:
        """下载生成的文件"""
        output_dir.mkdir(parents=True, exist_ok=True)
        downloaded = []

        result_files = response.get("ResultFile3Ds", [])
        for i, file3d in enumerate(result_files):
            file_type = file3d.get("Type", "unknown")
            url = file3d.get("Url", "")
            preview_url = file3d.get("PreviewImageUrl", "")

            # 下载 3D 模型
            if url:
                filename = f"model_{i+1}.{file_type.lower()}"
                filepath = output_dir / filename
                print(f"  📥 下载 {file_type}: {filename}")

                if not self.dry_run:
                    try:
                        dl = subprocess.run(
                            ["curl", "-sL", "-o", str(filepath), url],
                            capture_output=True, timeout=120,
                        )
                        if dl.returncode == 0 and filepath.exists():
                            size_kb = filepath.stat().st_size / 1024
                            print(f"    ✅ 已保存: {filepath} ({size_kb:.1f} KB)")
                            downloaded.append(str(filepath))
                        else:
                            print(f"    ❌ 下载失败")
                    except subprocess.TimeoutExpired:
                        print(f"    ❌ 下载超时")

            # 下载预览图
            if preview_url:
                preview_path = output_dir / f"preview_{i+1}.png"
                print(f"  📥 下载预览图: preview_{i+1}.png")

                if not self.dry_run:
                    try:
                        dl = subprocess.run(
                            ["curl", "-sL", "-o", str(preview_path), preview_url],
                            capture_output=True, timeout=60,
                        )
                        if dl.returncode == 0 and preview_path.exists():
                            print(f"    ✅ 已保存: {preview_path}")
                    except subprocess.TimeoutExpired:
                        pass

        return downloaded

    # ─── 完整流程 ───────────────────────────────────────────────────────────

    def generate_character(
        self,
        char_key: str,
        mode: str = "text",
        skip_rigging: bool = False,
        image_url: str | None = None,
        multiview: dict | None = None,
        model_url: str | None = None,
    ) -> dict:
        """
        完整生成单个角色的3D模型

        返回: {
            "char_key": str,
            "model_job_id": str | None,
            "model_files": list[str],
            "rigged_files": list[str],
            "status": "success" | "failed",
        }
        """
        char = CHARACTERS[char_key]
        output_dir = OUTPUT_DIR / char_key
        result = {
            "char_key": char_key,
            "name_cn": char["name_cn"],
            "model_job_id": None,
            "model_files": [],
            "rigged_files": [],
            "status": "failed",
        }

        print(f"\n{'='*60}")
        print(f"🎮 角色: {char['name_cn']} ({char['name_en']})")
        print(f"{'='*60}")

        # ─── Step 1: 生成3D模型 ───
        model_job_id = None
        model_response = None

        if mode in ("text", "image", "multiview"):
            gen_kwargs = {
                "enable_pbr": char["enable_pbr"],
                "face_count": char["face_count"],
                "model_version": char["model_version"],
                "generate_type": char["generate_type"],
                "result_format": char["result_format"],
            }

            if mode == "text":
                model_job_id = self.submit_text_to_3d(char["prompt"], **gen_kwargs)
            elif mode == "image":
                if not image_url:
                    print("❌ 图生3D 需要 --image-url")
                    return result
                model_job_id = self.submit_image_to_3d(image_url, **gen_kwargs)
            elif mode == "multiview":
                if not multiview:
                    print("❌ 多视角生3D 需要 --front/--back/--left/--right")
                    return result
                model_job_id = self.submit_multiview_to_3d(**multiview, **gen_kwargs)

            if not model_job_id:
                return result
            result["model_job_id"] = model_job_id

            # 等待完成
            model_response = self.wait_for_3d_job(model_job_id)
            if not model_response:
                return result

            # 下载模型
            result["model_files"] = self.download_results(model_response, output_dir)

        # ─── Step 2: 自动绑骨 ───
        if skip_rigging:
            print("\n⏭️  跳过绑骨")
        else:
            # 确定模型 URL（来自上一步结果或外部指定）
            rig_model_url = model_url
            rig_file_type = char.get("result_format", "FBX")
            if not rig_model_url and model_response:
                files = model_response.get("ResultFile3Ds", [])
                # 优先找 FBX 格式，其次 GLB，最后用第一个
                for f in files:
                    if f.get("Type") in ("FBX", "GLB"):
                        rig_model_url = f.get("Url")
                        rig_file_type = f.get("Type")
                        break
                if not rig_model_url and files:
                    rig_model_url = files[0].get("Url")
                    rig_file_type = files[0].get("Type", "FBX")

            if not rig_model_url:
                print("\n❌ 无可用于绑骨的模型 URL")
                return result

            motions = char.get("rig_motions", [])
            if motions:
                # 为每个动作生成绑骨
                for motion in motions:
                    motion_name = PRESET_MOTIONS.get(motion, str(motion))
                    print(f"\n--- 绑骨动作: {motion_name} ({motion}) ---")
                    rig_job_id = self.submit_auto_rigging(
                        rig_model_url,
                        file_type=rig_file_type,
                        motion_type=motion,
                    )
                    if rig_job_id:
                        rig_response = self.wait_for_rigging_job(rig_job_id)
                        if rig_response:
                            motion_dir = output_dir / "motions" / motion_name
                            rigged_files = self.download_results(rig_response, motion_dir)
                            result["rigged_files"].extend(rigged_files)
            else:
                # 只绑骨不带动作
                rig_job_id = self.submit_auto_rigging(
                    rig_model_url,
                    file_type=char["result_format"],
                )
                if rig_job_id:
                    rig_response = self.wait_for_rigging_job(rig_job_id)
                    if rig_response:
                        result["rigged_files"] = self.download_results(
                            rig_response, output_dir
                        )

        # 保存生成日志
        self._save_log(char_key, result)

        result["status"] = "success"
        print(f"\n✅ {char['name_cn']} 生成完成！")
        print(f"  📁 输出目录: {output_dir}")
        return result

    def _save_log(self, char_key: str, result: dict):
        """保存生成日志"""
        log_path = OUTPUT_DIR / char_key / "generation_log.json"
        log_path.parent.mkdir(parents=True, exist_ok=True)

        # 追加到日志列表
        logs = []
        if log_path.exists():
            try:
                logs = json.loads(log_path.read_text())
            except Exception:
                logs = []

        log_entry = {
            "timestamp": time.strftime("%Y-%m-%d %H:%M:%S"),
            **result,
        }
        logs.append(log_entry)
        log_path.write_text(json.dumps(logs, ensure_ascii=False, indent=2))
        print(f"  📝 日志已保存: {log_path}")


# ─── CLI ─────────────────────────────────────────────────────────────────────


def parse_args():
    parser = argparse.ArgumentParser(
        description="格斗萌主 3D 模型生成器 (tccli ai3d)",
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )

    # 角色选择
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument(
        "--character", "-c",
        choices=list(CHARACTERS.keys()),
        help="角色 key",
    )
    group.add_argument(
        "--all-characters",
        action="store_true",
        help="批量生成所有角色",
    )

    # 模式
    parser.add_argument(
        "--mode", "-m",
        choices=["text", "image", "multiview", "rig"],
        default="text",
        help="生成模式 (default: text)",
    )

    # 图片输入
    parser.add_argument("--image-url", help="图生3D 的图片 URL")
    parser.add_argument("--front", help="多视角: 正面图 URL")
    parser.add_argument("--back", help="多视角: 背面图 URL")
    parser.add_argument("--left", help="多视角: 左视图 URL")
    parser.add_argument("--right", help="多视角: 右视图 URL")
    parser.add_argument("--top", help="多视角: 顶视图 URL (仅3.1)")
    parser.add_argument("--bottom", help="多视角: 底视图 URL (仅3.1)")

    # 绑骨
    parser.add_argument("--model-url", help="绑骨用模型 URL (mode=rig 时必需)")
    parser.add_argument(
        "--motion", type=int, help="单个动作编号 (1-48)，不指定则用角色默认动作列表"
    )

    # 控制
    parser.add_argument("--skip-rigging", action="store_true", help="跳过绑骨步骤")
    parser.add_argument("--dry-run", action="store_true", help="干跑，不实际调用 API")
    parser.add_argument("--region", default=REGION, help="腾讯云区域")

    return parser.parse_args()


def main():
    args = parse_args()

    print("=" * 60)
    print("🎮 格斗萌主 — 3D 模型生成器")
    print(f"   模式: {args.mode} | 区域: {args.region}")
    if args.dry_run:
        print("   ⚠️  DRY-RUN 模式")
    print("=" * 60)

    pipeline = Hunyuan3DPipeline(region=args.region, dry_run=args.dry_run)

    # 构造多视角参数
    multiview = None
    if args.mode == "multiview":
        multiview = {
            "front": args.front,
            "back": args.back,
            "left": args.left,
            "right": args.right,
            "top": args.top,
            "bottom": args.bottom,
        }

    # 处理角色列表
    if args.all_characters:
        characters = list(CHARACTERS.keys())
    else:
        characters = [args.character]

    # 如果指定了 --motion，覆盖角色默认动作
    if args.motion is not None:
        for char_key in characters:
            CHARACTERS[char_key]["rig_motions"] = [args.motion]

    results = []
    for char_key in characters:
        result = pipeline.generate_character(
            char_key=char_key,
            mode=args.mode,
            skip_rigging=args.skip_rigging,
            image_url=args.image_url,
            multiview=multiview,
            model_url=args.model_url,
        )
        results.append(result)

    # 汇总
    print(f"\n{'='*60}")
    print("📊 生成汇总")
    print(f"{'='*60}")
    for r in results:
        status_icon = "✅" if r["status"] == "success" else "❌"
        print(
            f"  {status_icon} {r['name_cn']}: "
            f"模型 {len(r['model_files'])} 个, "
            f"绑骨 {len(r['rigged_files'])} 个"
        )

    success_count = sum(1 for r in results if r["status"] == "success")
    print(f"\n  总计: {success_count}/{len(results)} 成功")

    if success_count < len(results):
        sys.exit(1)


if __name__ == "__main__":
    main()

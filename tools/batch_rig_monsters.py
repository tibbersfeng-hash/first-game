#!/usr/bin/env python3
"""
批量补完绑骨脚本 — 为已生成 3D 模型的怪物补完剩余动作
用法:
  python3 batch_rig_monsters.py              # 补完所有怪物的剩余绑骨
  python3 batch_rig_monsters.py --monster candy_zombie  # 只处理指定怪物
"""

import argparse
import json
import sys
import time
from pathlib import Path

# 添加项目路径
PROJECT_ROOT = Path(__file__).parent.parent
sys.path.insert(0, str(PROJECT_ROOT / "tools"))

from generate_3d_models import (
    MONSTERS,
    Hunyuan3DPipeline,
    PRESET_MOTIONS,
    OUTPUT_DIR,
    REGION,
)


def get_completed_motions(monster_key: str) -> set:
    """获取已完成的绑骨动作"""
    motions_dir = OUTPUT_DIR / monster_key / "motions"
    completed = set()
    if motions_dir.exists():
        for motion_dir in motions_dir.iterdir():
            if motion_dir.is_dir():
                fbx_files = list(motion_dir.glob("*.fbx"))
                if fbx_files and fbx_files[0].stat().st_size > 1000:
                    completed.add(motion_dir.name)
    return completed


def get_model_url_from_log(monster_key: str) -> str | None:
    """从生成日志中获取模型 URL（需要重新生成来获取新 URL）"""
    # 由于 URL 可能已过期，我们需要重新提交 3D 生成来获取新 URL
    # 或者直接重新运行完整生成流程
    return None


def rig_monster_sequential(monster_key: str, pipeline: Hunyuan3DPipeline,
                           model_url: str, file_type: str = "FBX") -> dict:
    """为单个怪物串行执行所有绑骨动作"""
    monster = MONSTERS[monster_key]
    motions = monster.get("rig_motions", [])
    output_dir = OUTPUT_DIR / monster_key

    completed = get_completed_motions(monster_key)
    remaining = [m for m in motions if PRESET_MOTIONS.get(m) not in completed]

    print(f"\n{'='*60}")
    print(f"👾 {monster['name_cn']} ({monster['name_en']})")
    print(f"   已完成: {len(completed)}/{len(motions)} 个动作")
    print(f"   待完成: {len(remaining)} 个动作")
    print(f"{'='*60}")

    result = {"monster_key": monster_key, "rigged": [], "skipped": list(completed)}

    if not remaining:
        print("  ✅ 所有动作已完成，跳过")
        result["status"] = "done"
        return result

    for motion in remaining:
        motion_name = PRESET_MOTIONS.get(motion, str(motion))
        print(f"\n--- 绑骨动作: {motion_name} ({motion}) ---")

        rig_job_id = pipeline.submit_auto_rigging(
            model_url,
            file_type=file_type,
            motion_type=motion,
        )

        if not rig_job_id:
            print(f"  ❌ 提交失败，等待 30 秒后重试...")
            time.sleep(30)
            rig_job_id = pipeline.submit_auto_rigging(
                model_url,
                file_type=file_type,
                motion_type=motion,
            )
            if not rig_job_id:
                print(f"  ❌ 重试仍失败，跳过此动作")
                continue

        rig_response = pipeline.wait_for_rigging_job(rig_job_id)
        if rig_response:
            motion_dir = output_dir / "motions" / motion_name
            rigged_files = pipeline.download_results(rig_response, motion_dir)
            result["rigged"].extend(rigged_files)
            print(f"  ✅ {motion_name} 完成")
        else:
            print(f"  ❌ {motion_name} 失败")

        # 动作间间隔，避免 API 限流
        time.sleep(5)

    result["status"] = "done" if not remaining else "partial"
    return result


def main():
    parser = argparse.ArgumentParser(description="批量补完怪物绑骨")
    parser.add_argument("--monster", choices=list(MONSTERS.keys()),
                        help="指定怪物（不指定则处理所有）")
    parser.add_argument("--region", default=REGION, help="腾讯云区域")
    parser.add_argument("--dry-run", action="store_true", help="干跑")
    args = parser.parse_args()

    print("=" * 60)
    print("🔧 怪物绑骨补完工具")
    print(f"   区域: {args.region}")
    if args.dry_run:
        print("   ⚠️  DRY-RUN 模式")
    print("=" * 60)

    pipeline = Hunyuan3DPipeline(region=args.region, dry_run=args.dry_run)

    # 确定要处理的怪物列表
    if args.monster:
        monsters_to_process = [args.monster]
    else:
        # 处理所有有 3D 模型但未完成绑骨的怪物
        monsters_to_process = []
        for key in MONSTERS:
            model_fbx = OUTPUT_DIR / key / "model_3.fbx"
            if model_fbx.exists():
                completed = get_completed_motions(key)
                total = len(MONSTERS[key].get("rig_motions", []))
                if completed < total:
                    monsters_to_process.append(key)
                    print(f"  📋 {MONSTERS[key]['name_cn']}: {len(completed)}/{total} 动作")
                else:
                    print(f"  ✅ {MONSTERS[key]['name_cn']}: 全部完成")
            else:
                print(f"  ⏭️  {MONSTERS[key]['name_cn']}: 无 3D 模型，跳过")

    if not monsters_to_process:
        print("\n✅ 所有怪物绑骨已完成！")
        return

    print(f"\n📋 需要补完绑骨: {', '.join(monsters_to_process)}")

    # 串行处理每个怪物
    # 由于绑骨 API 只支持 1 路并发，必须串行
    for i, monster_key in enumerate(monsters_to_process):
        if i > 0:
            print(f"\n⏳ 等待 15 秒，避免 API 限流...")
            time.sleep(15)

        # 重新生成 3D 模型获取新 URL（因为 URL 会过期）
        monster = MONSTERS[monster_key]
        print(f"\n 重新生成 {monster['name_cn']} 3D 模型以获取绑骨 URL...")

        gen_kwargs = {
            "enable_pbr": monster["enable_pbr"],
            "face_count": monster["face_count"],
            "model_version": monster["model_version"],
            "generate_type": monster["generate_type"],
            "result_format": monster["result_format"],
        }

        # 文生3D 获取新 URL
        job_id = pipeline.submit_text_to_3d(monster["prompt"], **gen_kwargs)
        if not job_id:
            print(f"  ❌ 3D 生成提交失败")
            continue

        response = pipeline.wait_for_3d_job(job_id)
        if not response:
            print(f"  ❌ 3D 生成失败")
            continue

        # 获取 FBX URL
        model_url = None
        file_type = "FBX"
        for f in response.get("ResultFile3Ds", []):
            if f.get("Type") == "FBX":
                model_url = f.get("Url")
                file_type = "FBX"
                break
        if not model_url:
            for f in response.get("ResultFile3Ds", []):
                model_url = f.get("Url")
                file_type = f.get("Type", "FBX")
                break

        if not model_url:
            print(f"  ❌ 无法获取模型 URL")
            continue

        print(f"  ✅ 获取到模型 URL: {model_url[:60]}...")

        # 执行绑骨
        result = rig_monster_sequential(monster_key, pipeline, model_url, file_type)
        print(f"\n  📊 {monster['name_cn']}: 新增 {len(result['rigged'])} 个动作")

    print(f"\n{'='*60}")
    print("✅ 批量绑骨补完完成！")
    print(f"{'='*60}")


if __name__ == "__main__":
    main()

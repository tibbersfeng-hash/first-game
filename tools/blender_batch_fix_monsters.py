#!/usr/bin/env python3
"""
Blender batch post-process for 4 Candy Dungeon monsters.
Run headless:
    blender --background --python tools/blender_batch_fix_monsters.py

Pipeline:
    1. Import first motion FBX (contains rigged mesh + animation)
    2. Fix topology (normals, merge verts, remove interior faces)
    3. Fix bones (remove zero-length)
    4. Export clean mesh FBX (no animation)
    5. For each motion: re-import motion FBX, export individual anim FBX

Blender 4.0.2 compatible. Uses depsgraph context hacks for headless mode.
"""

import bpy
import os
import sys
import json
from pathlib import Path

# ---------------------------------------------------------------------------
# Config
# ---------------------------------------------------------------------------
PROJECT_ROOT = Path("/home/vipuser/first-game")
ASSET_ROOT = PROJECT_ROOT / "design" / "assets" / "output" / "3d"

MONSTERS = {
    "candy_zombie": {
        "output_name": "CandyZombie",
        "motions_dir": "motions",
        # Use first motion as the rigged mesh source
        "rigged_motion": "待机-1",
    },
    "gingerbread": {
        "output_name": "Gingerbread",
        "motions_dir": "motions",
        "rigged_motion": "待机-1",
    },
    "shadow_ninja": {
        "output_name": "ShadowNinja",
        "motions_dir": "motions",
        "rigged_motion": "待机-2",  # This monster uses 待机-2
    },
    "armored_gum": {
        "output_name": "ArmoredGum",
        "motions_dir": "motions",
        "rigged_motion": "待机-1",
    },
}

# Chinese motion folder -> UE5 animation name suffix
ANIM_MAP = {
    "待机": "Idle",
    "走路": "Walk",
    "慢跑": "Jog",
    "奔跑": "Run",
    "冲刺": "Sprint",
    "连续击打": "Attack",
    "击打": "Attack",
    "攻击": "Attack",
    "投掷": "Attack_Throw",
    "蓄力": "Attack_Charge",
    "回旋踢": "Attack_SpinKick",
    "受击倒地": "Death",
    "受击": "Hit",
}

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def log(msg: str):
    print(f"[batch_fix] {msg}", flush=True)


def force_object_mode():
    """Ensure we are in object mode (safe in background/headless)."""
    for obj in bpy.data.objects:
        if obj.mode != 'OBJECT':
            try:
                bpy.context.view_layer.objects.active = obj
                bpy.ops.object.mode_set(mode='OBJECT')
            except Exception:
                pass


def clear_scene():
    """Delete all objects and orphan data."""
    force_object_mode()
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete(use_global=False)

    # Clean orphan data
    for collection in [bpy.data.meshes, bpy.data.armatures,
                       bpy.data.materials, bpy.data.actions,
                       bpy.data.images, bpy.data.textures]:
        for block in collection:
            if block.users == 0:
                collection.remove(block)

    bpy.context.view_layer.update()


def import_fbx(fbx_path: str):
    """Import FBX with standard settings."""
    bpy.ops.import_scene.fbx(
        filepath=fbx_path,
        global_scale=1.0,
        use_prepost_rot=False,
        automatic_bone_orientation=True,
    )
    bpy.context.view_layer.update()


def get_mesh_and_armature():
    """Find the main mesh and armature objects in scene."""
    mesh_obj = None
    arm_obj = None
    for obj in bpy.data.objects:
        if obj.type == 'MESH' and mesh_obj is None:
            mesh_obj = obj
        elif obj.type == 'ARMATURE' and arm_obj is None:
            arm_obj = obj
    return mesh_obj, arm_obj


def fix_topology(mesh_obj):
    """Fix basic topology issues on a mesh object.

    NOTE: 'select_interior_faces' is intentionally skipped because
    AI-generated meshes often have non-manifold geometry that causes
    false positives (all faces selected as interior).
    """
    force_object_mode()
    bpy.context.view_layer.objects.active = mesh_obj
    bpy.ops.object.select_all(action='DESELECT')
    mesh_obj.select_set(True)

    # Enter edit mode
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.select_all(action='SELECT')

    # 1. Merge vertices by distance (remove doubles)
    result = bpy.ops.mesh.remove_doubles(threshold=0.001)
    log(f"    remove_doubles: {result}")

    # Re-select all after remove_doubles (selection may be cleared)
    bpy.ops.mesh.select_all(action='SELECT')

    # 2. Recalculate normals (wrapped for headless safety)
    try:
        bpy.ops.mesh.normals_recalculate()
        log(f"    normals_recalculate: OK")
    except Exception as e:
        log(f"    normals_recalculate via ops failed, using bmesh fallback")
        # Fallback: recalculate normals via bmesh
        import bmesh
        bm = bmesh.from_edit_mesh(mesh_obj.data)
        bmesh.ops.recalc_face_normals(bm, faces=bm.faces)
        bmesh.update_edit_mesh(mesh_obj.data)
        log(f"    bmesh recalc_face_normals: OK")

    # 3. Skip interior face deletion (unreliable on AI-generated meshes)
    # AI models with non-manifold geometry cause select_interior_faces
    # to mark ALL faces as interior, destroying the mesh.

    # 4. Final select all + normals pass
    bpy.ops.mesh.select_all(action='SELECT')
    try:
        bpy.ops.mesh.normals_recalculate()
    except Exception:
        pass

    # Back to object mode
    bpy.ops.object.mode_set(mode='OBJECT')
    bpy.context.view_layer.update()

    mesh_obj.select_set(False)


def add_lod_decimate(mesh_obj, ratio: float):
    """Add and apply a Decimate modifier at given ratio."""
    force_object_mode()
    bpy.context.view_layer.objects.active = mesh_obj
    bpy.ops.object.select_all(action='DESELECT')
    mesh_obj.select_set(True)

    # Add decimate modifier
    mod = mesh_obj.modifiers.new(name="Decimate_LOD", type='DECIMATE')
    mod.decimate_type = 'COLLAPSE'
    mod.ratio = ratio

    # Apply modifier
    bpy.ops.object.modifier_apply(modifier=mod.name)
    bpy.context.view_layer.update()

    mesh_obj.select_set(False)
    return len(mesh_obj.data.polygons)


def fix_bones(arm_obj):
    """Remove zero-length bones."""
    if arm_obj is None:
        return 0

    force_object_mode()
    bpy.context.view_layer.objects.active = arm_obj
    bpy.ops.object.select_all(action='DESELECT')
    arm_obj.select_set(True)
    bpy.ops.object.mode_set(mode='EDIT')

    armature = arm_obj.data
    bones_list = list(armature.edit_bones)
    removed = 0
    for bone in bones_list:
        length = (bone.head - bone.tail).length
        if length < 0.001:
            armature.edit_bones.remove(bone)
            removed += 1

    bpy.ops.object.mode_set(mode='OBJECT')
    bpy.context.view_layer.update()
    arm_obj.select_set(False)
    return removed


def collect_vertex_face_count(mesh_obj):
    """Return (vertex_count, face_count) for reporting."""
    return len(mesh_obj.data.vertices), len(mesh_obj.data.polygons)


def export_mesh_fbx(output_path: str):
    """Export the mesh (+ armature if present) as UE5-compatible FBX without animation."""
    mesh_obj, arm_obj = get_mesh_and_armature()
    if mesh_obj is None:
        log("  ❌ No mesh found, skipping export")
        return False

    force_object_mode()
    bpy.ops.object.select_all(action='DESELECT')
    mesh_obj.select_set(True)
    if arm_obj:
        arm_obj.select_set(True)
        bpy.context.view_layer.objects.active = arm_obj
    else:
        bpy.context.view_layer.objects.active = mesh_obj

    os.makedirs(os.path.dirname(output_path), exist_ok=True)

    has_armature = arm_obj is not None

    bpy.ops.export_scene.fbx(
        filepath=output_path,
        use_selection=True,
        global_scale=1.0,
        apply_unit_scale=True,
        apply_scale_options='FBX_SCALE_ALL',
        object_types={'ARMATURE', 'MESH'} if has_armature else {'MESH'},
        use_armature_deform_only=True,
        mesh_smooth_type='FACE',
        add_leaf_bones=False,
        primary_bone_axis='Y',
        secondary_bone_axis='X',
        bake_anim=False,
        path_mode='AUTO',
    )

    mesh_obj.select_set(False)
    if arm_obj:
        arm_obj.select_set(False)

    bpy.context.view_layer.update()
    return True


def export_anim_fbx(output_path: str):
    """Export current scene's active animation as UE5-compatible FBX."""
    mesh_obj, arm_obj = get_mesh_and_armature()
    if arm_obj is None:
        log("  ❌ No armature found, skipping anim export")
        return False

    force_object_mode()
    bpy.ops.object.select_all(action='DESELECT')
    mesh_obj.select_set(True)
    arm_obj.select_set(True)
    bpy.context.view_layer.objects.active = arm_obj

    os.makedirs(os.path.dirname(output_path), exist_ok=True)

    bpy.ops.export_scene.fbx(
        filepath=output_path,
        use_selection=True,
        global_scale=1.0,
        apply_unit_scale=True,
        apply_scale_options='FBX_SCALE_ALL',
        object_types={'ARMATURE', 'MESH'},
        use_armature_deform_only=True,
        mesh_smooth_type='FACE',
        add_leaf_bones=False,
        primary_bone_axis='Y',
        secondary_bone_axis='X',
        bake_anim=True,
        bake_anim_use_all_bones=True,
        bake_anim_use_nla_strips=False,
        bake_anim_use_all_actions=False,
        bake_anim_force_startend_keying=True,
        bake_anim_step=1.0,
        path_mode='AUTO',
    )

    mesh_obj.select_set(False)
    arm_obj.select_set(False)
    bpy.context.view_layer.update()
    return True


def map_motion_name(folder_name: str) -> str:
    """Map Chinese motion folder name to UE5 animation name suffix."""
    for cn_prefix, en_name in sorted(ANIM_MAP.items(), key=lambda x: -len(x[0])):
        if cn_prefix in folder_name:
            return en_name
    return folder_name.replace(" ", "_").replace("-", "_")


# ---------------------------------------------------------------------------
# Main pipeline per monster
# ---------------------------------------------------------------------------

def process_monster(name: str, config: dict) -> dict:
    """Process a single monster: import rigged mesh -> fix -> export mesh + anims."""
    monster_dir = ASSET_ROOT / name
    motions_dir = monster_dir / config["motions_dir"]
    output_dir = monster_dir / "ue5_ready"
    output_dir.mkdir(parents=True, exist_ok=True)

    result = {
        "name": name,
        "status": "ok",
        "verts": 0,
        "faces": 0,
        "bones": 0,
        "mesh_exported": False,
        "anims_exported": [],
        "errors": [],
    }

    log(f"\n{'='*60}")
    log(f"Processing: {name}")
    log(f"{'='*60}")

    # --- Phase 1: Import rigged mesh from first motion FBX ---
    rigged_motion = config["rigged_motion"]
    rigged_fbx = motions_dir / rigged_motion / "model_1.fbx"

    if not rigged_fbx.exists():
        # Fallback: try any available motion
        for d in sorted(motions_dir.iterdir()):
            candidate = d / "model_1.fbx"
            if candidate.exists():
                rigged_fbx = candidate
                rigged_motion = d.name
                break

    if not rigged_fbx.exists():
        result["status"] = "error"
        result["errors"].append(f"No rigged FBX found in {motions_dir}")
        return result

    clear_scene()
    log(f"  [1/4] Importing rigged mesh from: {rigged_motion}/model_1.fbx")
    try:
        import_fbx(str(rigged_fbx))
    except Exception as e:
        result["status"] = "error"
        result["errors"].append(f"Import failed: {e}")
        return result

    mesh_obj, arm_obj = get_mesh_and_armature()
    if mesh_obj is None:
        result["status"] = "error"
        result["errors"].append("No mesh after import")
        return result

    verts, faces = collect_vertex_face_count(mesh_obj)
    bones = len(arm_obj.data.bones) if arm_obj else 0
    mats = len(mesh_obj.data.materials)
    actions = len(bpy.data.actions)
    result["verts"] = verts
    result["faces"] = faces
    result["bones"] = bones
    log(f"  Raw stats: {verts} verts, {faces} faces, {bones} bones, {mats} materials, {actions} actions")

    # --- Phase 2: Fix topology ---
    log(f"  [2/4] Fixing topology...")
    try:
        fix_topology(mesh_obj)
        verts2, faces2 = collect_vertex_face_count(mesh_obj)
        log(f"  After fix: {verts2} verts, {faces2} faces (removed {verts - verts2} verts)")
        result["verts"] = verts2
        result["faces"] = faces2
    except Exception as e:
        result["errors"].append(f"Topology fix warning: {e}")
        log(f"  ⚠️  Topology issue: {e}")

    # --- Phase 3: Fix bones ---
    if arm_obj:
        log(f"  [3/4] Fixing bones...")
        try:
            removed = fix_bones(arm_obj)
            bones2 = len(arm_obj.data.bones)
            result["bones"] = bones2
            log(f"  Bones: {bones2} ({removed} zero-length removed)")
        except Exception as e:
            result["errors"].append(f"Bone fix warning: {e}")
            log(f"  ⚠️  Bone fix issue: {e}")
    else:
        log(f"  [3/4] No armature found, skipping bone fix")

    # --- Phase 4a: Export clean mesh (no anim) ---
    mesh_output = str(output_dir / f"SK_{config['output_name']}.fbx")
    log(f"  [4/4] Exporting mesh: {mesh_output}")
    if export_mesh_fbx(mesh_output):
        result["mesh_exported"] = True
        sz = os.path.getsize(mesh_output) / 1e6
        log(f"  ✅ Mesh exported: {sz:.1f} MB")
    else:
        result["errors"].append("Mesh export failed")

    # --- Phase 4b: Generate LODs via Decimate ---
    LOD_RATIOS = [
        ("LOD1", 0.5),   # 50% faces
        ("LOD2", 0.25),  # 25% faces
    ]
    result["lods_exported"] = []

    # Re-import the fixed mesh to generate LODs from clean geometry
    # (the current scene might have changed, so re-import the rigged source)
    clear_scene()
    try:
        import_fbx(str(rigged_fbx))
    except Exception as e:
        result["errors"].append(f"LOD re-import failed: {e}")
        log(f"  ⚠️  LOD generation skipped (re-import failed)")
        # Fall through to animation export

    mesh_obj_lod, arm_obj_lod = get_mesh_and_armature()
    if mesh_obj_lod and arm_obj_lod:
        # Apply topology fix again on fresh import
        try:
            fix_topology(mesh_obj_lod)
        except Exception:
            pass

        for lod_name, ratio in LOD_RATIOS:
            # Need to re-import for each LOD (decimate is destructive)
            if lod_name != "LOD1":
                clear_scene()
                try:
                    import_fbx(str(rigged_fbx))
                except Exception:
                    continue
                mesh_obj_lod, arm_obj_lod = get_mesh_and_armature()
                if not mesh_obj_lod:
                    continue
                try:
                    fix_topology(mesh_obj_lod)
                except Exception:
                    pass

            faces_before = len(mesh_obj_lod.data.polygons)
            faces_after = add_lod_decimate(mesh_obj_lod, ratio)

            lod_output = str(output_dir / f"SK_{config['output_name']}_{lod_name}.fbx")
            if export_mesh_fbx(lod_output):
                sz = os.path.getsize(lod_output) / 1e6
                result["lods_exported"].append(f"{lod_name} ({ratio:.0%}): {faces_after} faces")
                log(f"  ✅ {lod_name}: {faces_before} → {faces_after} faces ({sz:.1f} MB)")
            else:
                result["errors"].append(f"{lod_name} export failed")

    # --- Phase 4c: Export each animation individually ---
    motion_folders = sorted([d.name for d in motions_dir.iterdir() if d.is_dir()])
    log(f"\n  Exporting {len(motion_folders)} animations...")

    for motion_folder in motion_folders:
        motion_fbx = motions_dir / motion_folder / "model_1.fbx"
        if not motion_fbx.exists():
            result["errors"].append(f"Motion FBX missing: {motion_folder}")
            continue

        ue5_suffix = map_motion_name(motion_folder)
        anim_name = f"AM_{config['output_name']}_{ue5_suffix}"
        anim_output = str(output_dir / f"{anim_name}.fbx")

        # Import this motion FBX (has mesh + armature + one action)
        clear_scene()
        try:
            import_fbx(str(motion_fbx))
        except Exception as e:
            result["errors"].append(f"Motion import failed {motion_folder}: {e}")
            continue

        _, arm_m = get_mesh_and_armature()
        if arm_m is None:
            result["errors"].append(f"No armature in {motion_folder}")
            continue

        action_count = len(bpy.data.actions)
        if action_count == 0:
            result["errors"].append(f"No actions in {motion_folder}")
            continue

        # Log the action info
        for a in bpy.data.actions:
            log(f"    Action '{a.name}': frames {a.frame_range[0]:.0f}-{a.frame_range[1]:.0f}")

        # Export with animation
        if export_anim_fbx(anim_output):
            sz = os.path.getsize(anim_output) / 1e6
            result["anims_exported"].append(f"{motion_folder} -> {ue5_suffix}")
            log(f"  ✅ {motion_folder} -> {anim_name}.fbx ({sz:.1f} MB)")
        else:
            result["errors"].append(f"Anim export failed: {motion_folder}")

    # Done
    clear_scene()
    log(f"\n  === {name} Summary ===")
    log(f"  Mesh (LOD0): {'✅' if result['mesh_exported'] else '❌'}")
    if result.get('lods_exported'):
        for lod in result['lods_exported']:
            log(f"  LOD: {lod}")
    log(f"  Anims: {len(result['anims_exported'])}/6 exported")
    for a in result['anims_exported']:
        log(f"    {a}")
    if result['errors']:
        log(f"  Errors: {len(result['errors'])}")
        for e in result['errors']:
            log(f"    ⚠️  {e}")

    return result


def main():
    log("=" * 60)
    log("Blender Batch Monster Post-Processor")
    log(f"Blender version: {bpy.app.version_string}")
    log(f"Asset root: {ASSET_ROOT}")
    log(f"Monsters: {list(MONSTERS.keys())}")
    log("=" * 60)

    results = []
    for name, config in MONSTERS.items():
        result = process_monster(name, config)
        results.append(result)

    # Summary
    log("\n" + "=" * 60)
    log("FINAL SUMMARY")
    log("=" * 60)

    total_mesh = 0
    total_anims = 0
    all_ok = True

    for r in results:
        status = "✅" if r["status"] == "ok" and r["mesh_exported"] else "❌"
        log(f"  {status} {r['name']}: "
            f"{r['verts']}v/{r['faces']}f/{r['bones']}b, "
            f"mesh={'✅' if r['mesh_exported'] else '❌'}, "
            f"anims={len(r['anims_exported'])}/6")
        for a in r['anims_exported']:
            log(f"      {a}")
        if r["errors"]:
            all_ok = False
            for e in r["errors"]:
                log(f"      ⚠️  {e}")
        if r["mesh_exported"]:
            total_mesh += 1
        total_anims += len(r["anims_exported"])

    log(f"\n  Total: {total_mesh}/4 meshes, {total_anims}/24 animations")

    # Write results JSON
    results_path = str(ASSET_ROOT / "batch_fix_results.json")
    with open(results_path, 'w') as f:
        json.dump(results, f, indent=2, ensure_ascii=False)
    log(f"Results: {results_path}")

    if all_ok and total_mesh == 4 and total_anims == 24:
        log("\n🎉 All monsters processed successfully!")
    elif all_ok:
        log(f"\n⚠️  Partial success: {total_mesh}/4 meshes, {total_anims}/24 anims")
    else:
        log("\n❌ Some errors occurred - check log above")

    return 0 if (all_ok and total_mesh == 4) else 1


if __name__ == "__main__":
    sys.exit(main())

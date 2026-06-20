import unreal
import os
import sys

unreal.log('[INIT] Minimal init')

# 临时贴图导入钩子
if os.environ.get('FIRSTGAME_IMPORT_TEXTURES') == '1':
    unreal.log('[INIT] FIRSTGAME_IMPORT_TEXTURES detected, importing textures...')

    def import_textures():
        unreal.log('[IMPORT] Starting texture import...')

        # 导入 metallic 和 roughness 贴图
        textures_to_import = [
            {
                'source': '/home/vipuser/first-game/src_ue5/Content/Characters/Huikong/Mesh/model_3.fbm/texture_pbr_20250901_metallic.png',
                'name': 'T_Huikong_Metallic',
                'srgb': False
            },
            {
                'source': '/home/vipuser/first-game/src_ue5/Content/Characters/Huikong/Mesh/model_3.fbm/texture_pbr_20250901_roughness.png',
                'name': 'T_Huikong_Roughness',
                'srgb': False
            }
        ]

        task = unreal.AssetImportTask()
        task.automated = True
        task.save = True
        task.replace_existing = True

        for tex_info in textures_to_import:
            task.filename = tex_info['source']
            task.destination_path = '/Game/Characters/Huikong/Textures'

            options = unreal.TextureImportTaskOptions()
            options.compression_settings = unreal.TextureCompressionSettings.TC_DEFAULT
            options.s_rgb = tex_info['srgb']
            task.options = options

            unreal.log(f"[IMPORT] Importing {tex_info['name']}...")
            try:
                unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
                unreal.log(f"[IMPORT] ✅ {tex_info['name']} imported")
            except Exception as e:
                unreal.log_error(f"[IMPORT] ❌ Failed: {e}")

        unreal.log('[IMPORT] Texture import complete')
        unreal.SystemLibrary.execute_console_command(None, 'quit')

    import_textures()

# 创建 NPR 材质钩子
elif os.environ.get('FIRSTGAME_CREATE_NPR_MATERIAL') == '1':
    unreal.log('[INIT] FIRSTGAME_CREATE_NPR_MATERIAL detected, creating NPR material...')

    def create_npr_material():
        unreal.log('[MATERIAL] Creating NPR cartoon material...')

        # 创建材质
        material_factory = unreal.MaterialFactoryNew()
        material_path = '/Game/Characters/Huikong/Materials'
        material_name = 'M_Huikong_NPR'

        # 检查目录是否存在，不存在则创建
        if not unreal.EditorAssetLibrary.does_directory_exist(material_path):
            unreal.EditorAssetLibrary.make_directory(material_path)
            unreal.log(f'[MATERIAL] Created directory: {material_path}')

        full_material_path = f'{material_path}/{material_name}'

        # 创建材质资产
        material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            asset_name=material_name,
            package_path=material_path,
            asset_class=unreal.Material,
            factory=material_factory
        )

        if not material:
            unreal.log_error('[MATERIAL] ❌ Failed to create material')
            return

        unreal.log(f'[MATERIAL] ✅ Created material: {full_material_path}')

        # 保存材质
        try:
            unreal.EditorAssetLibrary.save_asset(full_material_path)
            unreal.log('[MATERIAL] ✅ Material saved')
        except Exception as e:
            unreal.log_error(f'[MATERIAL] ❌ Save failed: {e}')

        unreal.SystemLibrary.execute_console_command(None, 'quit')

    create_npr_material()

# 临时验证钩子：检查环境变量 FIRSTGAME_VERIFY_ASSETS
elif os.environ.get('FIRSTGAME_VERIFY_ASSETS') == '1':
    unreal.log('[INIT] FIRSTGAME_VERIFY_ASSETS detected, running asset verification...')

    def verify_assets():
        import time

        unreal.log('[VERIFY] === Asset Verification Started ===')

        # 验证骨骼网格体
        try:
            skm = unreal.load_asset('/Game/Characters/Huikong/Mesh/SKM_Huikong')
            if skm:
                unreal.log('[VERIFY] ✅ SKM_Huikong (Skeletal Mesh) loaded')
                unreal.log(f'[VERIFY]    - Skeleton: {skm.skeleton}')
                unreal.log(f'[VERIFY]    - Materials: {len(skm.materials)}')
            else:
                unreal.log_error('[VERIFY] ❌ SKM_Huikong not found')
        except Exception as e:
            unreal.log_error(f'[VERIFY] ❌ SKM_Huikong error: {e}')

        # 验证动画
        anims = ['Idle_01', 'Idle_02', 'Walk_01', 'Run_01', 'LightAttack_01', 'HitReaction', 'Landing']
        for anim_name in anims:
            try:
                anim = unreal.load_asset(f'/Game/Characters/Huikong/Animations/AM_Huikong_{anim_name}')
                if anim:
                    unreal.log(f'[VERIFY] ✅ AM_Huikong_{anim_name} loaded')
                else:
                    unreal.log_error(f'[VERIFY] ❌ AM_Huikong_{anim_name} not found')
            except Exception as e:
                unreal.log_error(f'[VERIFY] ❌ AM_Huikong_{anim_name} error: {e}')

        # 验证动画蓝图
        try:
            abp = unreal.load_asset('/Game/Characters/Huikong/ABP/ABP_Huikong')
            if abp:
                unreal.log('[VERIFY] ✅ ABP_Huikong (Animation Blueprint) loaded')
            else:
                unreal.log_error('[VERIFY] ❌ ABP_Huikong not found')
        except Exception as e:
            unreal.log_error(f'[VERIFY] ❌ ABP_Huikong error: {e}')

        unreal.log('[VERIFY] === Asset Verification Complete ===')

        # 退出编辑器
        unreal.SystemLibrary.execute_console_command(None, 'quit')

    # 使用延迟执行，等待编辑器完全加载
    # 注意：不能在线程中调用 Unreal API，必须使用定时器或延迟执行
    unreal.SystemLibrary.execute_console_command(None, 'obj dump SKM_Huikong')
    verify_assets()

# ── Monster import hook ──────────────────────────────────────────────
elif os.environ.get('FIRSTGAME_IMPORT_MONSTERS') == '1':
    unreal.log('[INIT] FIRSTGAME_IMPORT_MONSTERS detected, importing candy dungeon monsters...')
    import import_monsters

# ── Monster material + AnimBP setup hook ─────────────────────────────
elif os.environ.get('FIRSTGAME_SETUP_MONSTERS') == '1':
    unreal.log('[INIT] FIRSTGAME_SETUP_MONSTERS detected, creating materials + AnimBPs...')
    import setup_monsters

# ── Monster BlendSpace + AnimBP config hook ──────────────────────────
elif os.environ.get('FIRSTGAME_SETUP_ANIMBP') == '1':
    unreal.log('[INIT] FIRSTGAME_SETUP_ANIMBP detected, configuring BlendSpaces + AnimBPs...')
    import setup_animbp

# ── Monster test level hook ──────────────────────────────────────────
elif os.environ.get('FIRSTGAME_TEST_LEVEL') == '1':
    unreal.log('[INIT] FIRSTGAME_TEST_LEVEL detected, creating monster test level...')
    import setup_test_level_monsters

# ── Enemy Blueprint creation hook ────────────────────────────────────
elif os.environ.get('FIRSTGAME_CREATE_ENEMY_BPS') == '1':
    unreal.log('[INIT] FIRSTGAME_CREATE_ENEMY_BPS detected, creating enemy BPs...')
    import create_enemy_bps

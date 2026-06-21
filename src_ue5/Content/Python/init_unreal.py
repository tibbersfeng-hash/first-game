import unreal
import os
import sys

unreal.log('[INIT] Minimal init')

# 临时贴图导入钩子
if os.environ.get('FIRSTGAME_IMPORT_TEXTURES') == '1':
    unreal.log('[INIT] FIRSTGAME_IMPORT_TEXTURES detected, importing textures...')
    import import_textures

# 创建 NPR 材质钩子
elif os.environ.get('FIRSTGAME_CREATE_NPR_MATERIAL') == '1':
    unreal.log('[INIT] FIRSTGAME_CREATE_NPR_MATERIAL detected, creating NPR material...')
    import create_npr_material

# 验证资产钩子
elif os.environ.get('FIRSTGAME_VERIFY_ASSETS') == '1':
    unreal.log('[INIT] FIRSTGAME_VERIFY_ASSETS detected, running asset verification...')
    import verify_assets

# Monster import hook
elif os.environ.get('FIRSTGAME_IMPORT_MONSTERS') == '1':
    unreal.log('[INIT] FIRSTGAME_IMPORT_MONSTERS detected, importing candy dungeon monsters...')
    import import_monsters

# Monster material + AnimBP setup hook
elif os.environ.get('FIRSTGAME_SETUP_MONSTERS') == '1':
    unreal.log('[INIT] FIRSTGAME_SETUP_MONSTERS detected, creating materials + AnimBPs...')
    import setup_monsters

# Monster BlendSpace + AnimBP config hook
elif os.environ.get('FIRSTGAME_SETUP_ANIMBP') == '1':
    unreal.log('[INIT] FIRSTGAME_SETUP_ANIMBP detected, configuring BlendSpaces + AnimBPs...')
    import setup_animbp

# Monster test level hook
elif os.environ.get('FIRSTGAME_TEST_LEVEL') == '1':
    unreal.log('[INIT] FIRSTGAME_TEST_LEVEL detected, creating monster test level...')
    import setup_test_level_monsters

# Enemy Blueprint creation hook
elif os.environ.get('FIRSTGAME_CREATE_ENEMY_BPS') == '1':
    unreal.log('[INIT] FIRSTGAME_CREATE_ENEMY_BPS detected, creating enemy BPs...')
    import create_enemy_bps

# NPR colored material instances hook
elif os.environ.get('FIRSTGAME_NPR_MATERIALS') == '1':
    unreal.log('[INIT] FIRSTGAME_NPR_MATERIALS detected, creating NPR materials...')
    import create_npr_materials

# LOD configuration hook
elif os.environ.get('FIRSTGAME_CONFIG_LODS') == '1':
    unreal.log('[INIT] FIRSTGAME_CONFIG_LODS detected, configuring LODs...')
    import configure_lods

# Spawn monsters + screenshot hook
elif os.environ.get('FIRSTGAME_SPAWN_AND_SHOT') == '1':
    unreal.log('[INIT] FIRSTGAME_SPAWN_AND_SHOT detected, spawning + screenshot...')
    import spawn_and_screenshot

# Better spawn + lighting + screenshot hook
elif os.environ.get('FIRSTGAME_BETTER_SHOT') == '1':
    unreal.log('[INIT] FIRSTGAME_BETTER_SHOT detected, better spawn + screenshot...')
    import spawn_better_shot

# Enemy spawn test hook
elif os.environ.get('FIRSTGAME_SPAWN_ENEMY_TEST') == '1':
    unreal.log('[INIT] FIRSTGAME_SPAWN_ENEMY_TEST detected, spawning enemies...')
    import spawn_enemy_test

# NPR material test (spawn + apply NPR + screenshot)
elif os.environ.get('FIRSTGAME_NPR_TEST') == '1':
    unreal.log('[INIT] FIRSTGAME_NPR_TEST detected, testing NPR materials...')
    import test_npr_material

# Create NPR material assets (Unlit parent + MIC per monster)
elif os.environ.get('FIRSTGAME_CREATE_NPR_ASSET') == '1':
    unreal.log('[INIT] FIRSTGAME_CREATE_NPR_ASSET detected, creating NPR material assets...')
    import create_npr_asset

# Build NPR material with full expression graph via MaterialEditingLibrary
elif os.environ.get('FIRSTGAME_BUILD_NPR_MATERIAL') == '1':
    unreal.log('[INIT] FIRSTGAME_BUILD_NPR_MATERIAL detected, building NPR material...')
    import build_npr_material

# Fix NPR material connection
elif os.environ.get('FIRSTGAME_FIX_NPR_MATERIAL') == '1':
    unreal.log('[INIT] FIRSTGAME_FIX_NPR_MATERIAL detected, fixing NPR material...')
    import fix_npr_material

# Final NPR solution with shader compilation
elif os.environ.get('FIRSTGAME_FINAL_NPR') == '1':
    unreal.log('[INIT] FIRSTGAME_FINAL_NPR detected, running final NPR solution...')
    import final_npr_solution

# Integration test
elif os.environ.get('FIRSTGAME_INTEGRATION_TEST') == '1':
    unreal.log('[INIT] FIRSTGAME_INTEGRATION_TEST detected, running integration test...')
    import integration_test

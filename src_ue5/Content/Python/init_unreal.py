import unreal
import os
import sys

unreal.log('[INIT] Minimal init')

# 临时验证钩子：检查环境变量 FIRSTGAME_VERIFY_ASSETS
if os.environ.get('FIRSTGAME_VERIFY_ASSETS') == '1':
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

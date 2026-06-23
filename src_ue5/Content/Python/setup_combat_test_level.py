import unreal

def setup_combat_test_level():
    """设置战斗测试关卡"""
    
    unreal.log('=== 开始设置战斗测试关卡 ===')
    
    # 1. 创建新关卡
    level_path = '/Game/Maps/TestLevel_Combat_Auto'
    if not unreal.EditorAssetLibrary.does_asset_exist(level_path):
        unreal.EditorLevelLibrary().save_current_level()
        unreal.EditorLevelLibrary().new_level(level_path)
        unreal.log(f'✅ 创建关卡: {level_path}')
    else:
        unreal.EditorLevelLibrary().load_level(level_path)
        unreal.log(f'📂 加载关卡: {level_path}')
    
    # 2. 设置光照
    # 方向光
    directional_light = unreal.EditorLevelLibrary().spawn_actor_from_class(
        unreal.DirectionalLight,
        unreal.Vector(0, 0, 1000),
        unreal.Rotator(-45, 0, 0)
    )
    directional_light.set_actor_label('SunLight')
    directional_light.set_intensity(5000)
    unreal.log('✅ 创建方向光')
    
    # 天光
    sky_light = unreal.EditorLevelLibrary().spawn_actor_from_class(
        unreal.SkyLight,
        unreal.Vector(0, 0, 1500),
        unreal.Rotator(0, 0, 0)
    )
    sky_light.set_actor_label('SkyLight')
    unreal.log('✅ 创建天光')
    
    # 3. 生成测试怪物
    monsters = [
        ('CandyZombie', unreal.Vector(-500, 0, 0)),
        ('Gingerbread', unreal.Vector(500, 0, 0)),
        ('ShadowNinja', unreal.Vector(0, -500, 0)),
        ('ArmoredGum', unreal.Vector(0, 500, 0))
    ]
    
    for monster_name, location in monsters:
        # 生成 BaseEnemy Actor
        enemy = unreal.EditorLevelLibrary().spawn_actor_from_class(
            unreal.load_class(None, '/Script/FirstGame.BaseEnemy'),
            location,
            unreal.Rotator(0, 0, 0)
        )
        enemy.set_actor_label(f'TestEnemy_{monster_name}')
        
        # 设置怪物类型（通过 C++ 属性）
        enemy_type_enum = unreal.load_object(unreal.Enum, '/Script/FirstGame.EEnemyType')
        if monster_name == 'CandyZombie':
            enemy.set_editor_property('EnemyType', enemy_type_enum.CANDY_ZOMBIE)
        elif monster_name == 'Gingerbread':
            enemy.set_editor_property('EnemyType', enemy_type_enum.GINGERBREAD)
        elif monster_name == 'ShadowNinja':
            enemy.set_editor_property('EnemyType', enemy_type_enum.SHADOW_NINJA)
        elif monster_name == 'ArmoredGum':
            enemy.set_editor_property('EnemyType', enemy_type_enum.ARMORED_GUM)
        
        unreal.log(f'✅ 生成测试怪物: {monster_name}')
    
    # 4. 生成玩家角色
    player = unreal.EditorLevelLibrary().spawn_actor_from_class(
        unreal.load_class(None, '/Script/FirstGame.PlayerCharacter'),
        unreal.Vector(0, 0, 100),
        unreal.Rotator(0, 0, 0)
    )
    player.set_actor_label('TestPlayer')
    unreal.log('✅ 生成玩家角色')
    
    # 5. 保存关卡
    unreal.EditorLevelLibrary().save_current_level()
    unreal.log('✅ 关卡已保存')
    
    unreal.log('=== 战斗测试关卡设置完成 ===')
    unreal.log('提示: 按 Alt+P 进入 PIE 模式测试')

if __name__ == '__main__':
    setup_combat_test_level()

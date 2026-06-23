import unreal

def create_outline_material_instances():
    """为 4 个怪物创建描边材质实例"""
    
    # 怪物列表和颜色
    monsters = [
        ('CandyZombie', unreal.Color(0, 255, 0, 255)),      # 绿色
        ('Gingerbread', unreal.Color(139, 69, 19, 255)),    # 棕色
        ('ShadowNinja', unreal.Color(128, 0, 128, 255)),    # 紫色
        ('ArmoredGum', unreal.Color(192, 192, 192, 255))    # 银白色
    ]
    
    asset_tools = unreal.AssetTools()
    
    for monster_name, outline_color in monsters:
        # 创建材质实例
        instance_name = f'MI_{monster_name}_Outline'
        instance_path = f'/Game/Monsters/{monster_name}/Materials/{instance_name}'
        
        try:
            # 检查是否已存在
            if unreal.EditorAssetLibrary.does_asset_exist(instance_path):
                unreal.log(f'材质实例已存在: {instance_name}')
                continue
            
            # 创建材质实例（基于 M_Outline 父材质）
            parent_material = unreal.load_asset('/Game/Materials/M_Outline')
            if not parent_material:
                unreal.log_warning(f'父材质 M_Outline 不存在，跳过 {monster_name}')
                continue
            
            instance = asset_tools.create_asset(
                asset_name=instance_name,
                package_path=f'/Game/Monsters/{monster_name}/Materials',
                asset_class=unreal.MaterialInstanceConstant,
                factory=unreal.MaterialInstanceConstantFactoryNew()
            )
            
            # 设置父材质
            instance.set_editor_property('parent', parent_material)
            
            # 设置描边颜色参数
            instance.set_vector_parameter_value('OutlineColor', outline_color)
            
            # 保存资产
            unreal.EditorAssetLibrary.save_asset(instance_path)
            unreal.log(f'✅ 创建描边材质: {instance_name}')
            
        except Exception as e:
            unreal.log_error(f'❌ 创建失败 {monster_name}: {str(e)}')

if __name__ == '__main__':
    create_outline_material_instances()

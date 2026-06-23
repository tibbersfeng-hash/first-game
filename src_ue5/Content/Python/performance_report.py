import unreal
import time

def generate_performance_report():
    """生成性能报告"""
    
    unreal.log('=== 开始性能分析 ===')
    
    report = {
        'timestamp': time.strftime('%Y-%m-%d %H:%M:%S'),
        'assets': {},
        'memory': {},
        'performance': {}
    }
    
    # 1. 统计资产
    asset_registry = unreal.AssetRegistry()
    all_assets = asset_registry.get_assets()
    
    asset_counts = {}
    for asset in all_assets:
        asset_class = str(asset.asset_class)
        asset_counts[asset_class] = asset_counts.get(asset_class, 0) + 1
    
    report['assets'] = asset_counts
    unreal.log(f'📊 资产统计: {len(all_assets)} 个')
    
    # 2. 内存使用
    # 注意: UE5 Python API 获取内存信息有限
    report['memory']['note'] = '使用 Unreal Insights 获取详细内存数据'
    
    # 3. 场景统计
    all_actors = unreal.EditorLevelLibrary().get_all_level_actors()
    actor_counts = {}
    for actor in all_actors:
        actor_class = str(actor.get_class())
        actor_counts[actor_class] = actor_counts.get(actor_class, 0) + 1
    
    report['performance']['actor_count'] = len(all_actors)
    report['performance']['actor_types'] = actor_counts
    
    unreal.log(f'🎭 Actor 统计: {len(all_actors)} 个')
    
    # 4. 输出报告
    unreal.log('=== 性能报告 ===')
    for key, value in report.items():
        unreal.log(f'{key}: {value}')
    
    return report

if __name__ == '__main__':
    generate_performance_report()

import os
import json

def scan_assets(root_path):
    """扫描所有 UE5 资产"""
    assets = {
        'uasset': [],
        'umap': [],
        'fbx': [],
        'png': [],
        'cpp': [],
        'h': []
    }
    
    for root, dirs, files in os.walk(root_path):
        # 跳过 Intermediate 和 Build 目录
        if 'Intermediate' in root or 'Build' in root or '.git' in root:
            continue
            
        for file in files:
            ext = os.path.splitext(file)[1].lower()
            full_path = os.path.join(root, file)
            rel_path = os.path.relpath(full_path, root_path)
            size = os.path.getsize(full_path)
            
            if ext == '.uasset':
                assets['uasset'].append({'path': rel_path, 'size': size})
            elif ext == '.umap':
                assets['umap'].append({'path': rel_path, 'size': size})
            elif ext == '.fbx':
                assets['fbx'].append({'path': rel_path, 'size': size})
            elif ext == '.png':
                assets['png'].append({'path': rel_path, 'size': size})
            elif ext == '.cpp':
                assets['cpp'].append({'path': rel_path, 'size': size})
            elif ext == '.h':
                assets['h'].append({'path': rel_path, 'size': size})
    
    return assets

def format_size(size):
    """格式化文件大小"""
    for unit in ['B', 'KB', 'MB', 'GB']:
        if size < 1024.0:
            return f"{size:.1f} {unit}"
        size /= 1024.0
    return f"{size:.1f} TB"

# 扫描项目
project_root = r'D:\workspace\first-game\src_ue5'
assets = scan_assets(project_root)

# 生成报告
report = {
    'summary': {
        'uasset_count': len(assets['uasset']),
        'umap_count': len(assets['umap']),
        'fbx_count': len(assets['fbx']),
        'png_count': len(assets['png']),
        'cpp_count': len(assets['cpp']),
        'h_count': len(assets['h'])
    },
    'assets': assets
}

# 计算总大小
total_size = sum(a['size'] for category in assets.values() for a in category)
report['summary']['total_size'] = format_size(total_size)

# 保存到文件
output_path = os.path.join(project_root, '..', 'docs', 'asset_report.json')
os.makedirs(os.path.dirname(output_path), exist_ok=True)
with open(output_path, 'w', encoding='utf-8') as f:
    json.dump(report, f, indent=2, ensure_ascii=False)

# 打印摘要
print(f"=== 资产报告 ===")
print(f"UE5 资产 (.uasset): {len(assets['uasset'])} 个")
print(f"关卡 (.umap): {len(assets['umap'])} 个")
print(f"FBX 模型: {len(assets['fbx'])} 个")
print(f"贴图 (.png): {len(assets['png'])} 个")
print(f"C++ 源文件: {len(assets['cpp'])} 个")
print(f"C++ 头文件: {len(assets['h'])} 个")
print(f"总大小: {report['summary']['total_size']}")
print(f"\n报告已保存到: {output_path}")

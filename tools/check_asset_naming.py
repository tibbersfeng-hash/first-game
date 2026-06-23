#!/usr/bin/env python3
"""
资产命名规范检查器
检查 UE5 资产是否遵循命名约定
"""

import os
import re
from pathlib import Path

def check_asset_name(filepath):
    """检查单个资产的命名"""
    filename = os.path.basename(filepath)
    name_without_ext = os.path.splitext(filename)[0]
    
    issues = []
    
    # 检查是否包含空格
    if ' ' in name_without_ext:
        issues.append('包含空格')
    
    # 检查是否包含特殊字符
    if not re.match(r'^[A-Za-z0-9_]+$', name_without_ext):
        issues.append('包含非法字符')
    
    # 检查是否以下划线开头或结尾
    if name_without_ext.startswith('_') or name_without_ext.endswith('_'):
        issues.append('以下划线开头或结尾')
    
    # 检查是否是大驼峰命名
    if name_without_ext and not name_without_ext[0].isupper():
        issues.append('未使用大驼峰命名')
    
    return issues

def scan_assets():
    """扫描所有资产"""
    content_dir = Path(r"D:\workspace\first-game\src_ue5\Content")
    
    issues = []
    checked = 0
    
    for ext in ['*.uasset', '*.umap']:
        for filepath in content_dir.rglob(ext):
            checked += 1
            name_issues = check_asset_name(filepath)
            
            if name_issues:
                issues.append({
                    'file': str(filepath.relative_to(content_dir)),
                    'issues': name_issues
                })
    
    # 打印报告
    print("="*60)
    print("资产命名规范检查报告")
    print("="*60)
    print(f"\n检查文件数: {checked}")
    print(f"问题文件数: {len(issues)}")
    
    if issues:
        print("\n问题详情 (前 20 个):")
        for issue in issues[:20]:
            print(f"\n  {issue['file']}")
            for problem in issue['issues']:
                print(f"    - {problem}")
        
        if len(issues) > 20:
            print(f"\n  ... 还有 {len(issues) - 20} 个问题")
    else:
        print("\n✅ 所有资产命名符合规范！")
    
    return issues

if __name__ == '__main__':
    scan_assets()

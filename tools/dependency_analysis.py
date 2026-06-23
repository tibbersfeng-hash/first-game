#!/usr/bin/env python3
"""
C++ 依赖关系分析工具
分析模块间的依赖关系
"""

import os
import re
from collections import defaultdict
from pathlib import Path

def extract_includes(filepath):
    """提取文件中的 #include"""
    includes = []
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        for line in f:
            match = re.match(r'#include\s+[<"]([^>"]+)[>"]', line.strip())
            if match:
                includes.append(match.group(1))
    return includes

def analyze_dependencies():
    """分析项目依赖"""
    source_dir = Path(r"D:\workspace\first-game\src_ue5\Source\FirstGame")
    
    # 模块依赖
    module_deps = defaultdict(set)
    
    # 文件依赖
    file_deps = {}
    
    for ext in ['*.cpp', '*.h']:
        for filepath in source_dir.rglob(ext):
            rel_path = filepath.relative_to(source_dir)
            includes = extract_includes(filepath)
            
            # 分类 includes
            for inc in includes:
                # 项目内部依赖
                if inc.startswith('FirstGame/') or inc.startswith('Private/') or inc.startswith('Public/'):
                    # 提取模块名
                    parts = inc.split('/')
                    if len(parts) > 1:
                        module = parts[1] if parts[0] in ['Private', 'Public'] else parts[0]
                        module_deps[str(rel_path.parent)].add(module)
                
                # UE 引擎依赖
                elif inc.startswith('Engine/') or inc.startswith('Runtime/') or inc.startswith('Developer/'):
                    module_deps[str(rel_path.parent)].add('UE_Engine')
            
            file_deps[str(rel_path)] = includes
    
    # 打印报告
    print("="*60)
    print("模块依赖关系分析")
    print("="*60)
    
    print("\n模块间依赖:")
    for module, deps in sorted(module_deps.items()):
        if module != '.':
            print(f"\n  {module}/")
            for dep in sorted(deps):
                if dep != module:
                    print(f"    -> {dep}")
    
    # 统计 UE 引擎依赖
    ue_deps = set()
    for deps in module_deps.values():
        if 'UE_Engine' in deps:
            ue_deps.update(deps)
    
    print(f"\n\nUE 引擎模块依赖数: {len(ue_deps)}")
    
    return module_deps

if __name__ == '__main__':
    analyze_dependencies()

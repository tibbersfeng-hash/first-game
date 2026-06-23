#!/usr/bin/env python3
"""
代码质量分析工具
分析 C++ 代码的复杂度、依赖关系等
"""

import os
import re
from collections import defaultdict
from pathlib import Path

def analyze_cpp_file(filepath):
    """分析单个 C++ 文件"""
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()
    
    lines = content.split('\n')
    
    metrics = {
        'total_lines': len(lines),
        'code_lines': 0,
        'comment_lines': 0,
        'blank_lines': 0,
        'functions': 0,
        'classes': 0,
        'includes': [],
        'includes_ue': [],
        'includes_std': []
    }
    
    in_comment = False
    for line in lines:
        stripped = line.strip()
        
        # 跳过空行
        if not stripped:
            metrics['blank_lines'] += 1
            continue
        
        # 多行注释
        if '/*' in stripped:
            in_comment = True
        if '*/' in stripped:
            in_comment = False
            metrics['comment_lines'] += 1
            continue
        
        if in_comment or stripped.startswith('//'):
            metrics['comment_lines'] += 1
            continue
        
        metrics['code_lines'] += 1
        
        # 检测函数
        if re.search(r'\b\w+\s*\([^)]*\)\s*{', stripped):
            metrics['functions'] += 1
        
        # 检测类
        if re.search(r'\bclass\s+\w+', stripped):
            metrics['classes'] += 1
        
        # 检测 includes
        include_match = re.match(r'#include\s+[<"]([^>"]+)[>"]', stripped)
        if include_match:
            include = include_match.group(1)
            metrics['includes'].append(include)
            if include.startswith('Engine/') or include.startswith('Runtime/'):
                metrics['includes_ue'].append(include)
            elif include.startswith('<') or '/' not in include:
                metrics['includes_std'].append(include)
    
    return metrics

def analyze_project():
    """分析整个项目"""
    source_dir = Path(r"D:\workspace\first-game\src_ue5\Source")
    
    all_metrics = []
    total_files = 0
    
    # 分析所有 C++ 文件
    for ext in ['*.cpp', '*.h']:
        for filepath in source_dir.rglob(ext):
            metrics = analyze_cpp_file(filepath)
            metrics['file'] = str(filepath.relative_to(source_dir))
            all_metrics.append(metrics)
            total_files += 1
    
    # 汇总统计
    summary = {
        'total_files': total_files,
        'total_lines': sum(m['total_lines'] for m in all_metrics),
        'total_code_lines': sum(m['code_lines'] for m in all_metrics),
        'total_comment_lines': sum(m['comment_lines'] for m in all_metrics),
        'total_functions': sum(m['functions'] for m in all_metrics),
        'total_classes': sum(m['classes'] for m in all_metrics),
        'avg_lines_per_file': sum(m['total_lines'] for m in all_metrics) / total_files if total_files > 0 else 0,
        'comment_ratio': sum(m['comment_lines'] for m in all_metrics) / sum(m['code_lines'] for m in all_metrics) if sum(m['code_lines'] for m in all_metrics) > 0 else 0
    }
    
    # 找出最大的文件
    largest_files = sorted(all_metrics, key=lambda x: x['total_lines'], reverse=True)[:10]
    
    # 找出函数最多的文件
    most_functions = sorted(all_metrics, key=lambda x: x['functions'], reverse=True)[:10]
    
    # 打印报告
    print("="*60)
    print("代码质量分析报告")
    print("="*60)
    print(f"\n总文件数: {summary['total_files']}")
    print(f"总行数: {summary['total_lines']}")
    print(f"代码行数: {summary['total_code_lines']}")
    print(f"注释行数: {summary['total_comment_lines']}")
    print(f"注释比例: {summary['comment_ratio']:.1%}")
    print(f"总函数数: {summary['total_functions']}")
    print(f"总类数: {summary['total_classes']}")
    print(f"平均每文件行数: {summary['avg_lines_per_file']:.0f}")
    
    print("\n最大的 10 个文件:")
    for m in largest_files:
        print(f"  {m['file']}: {m['total_lines']} 行")
    
    print("\n函数最多的 10 个文件:")
    for m in most_functions:
        print(f"  {m['file']}: {m['functions']} 个函数")
    
    return summary

if __name__ == '__main__':
    analyze_project()

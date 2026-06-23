#!/usr/bin/env python3
"""
变更日志生成器
从 Git 提交历史生成 CHANGELOG.md
"""

import subprocess
import re
from datetime import datetime
from pathlib import Path

def get_git_log(since=None):
    """获取 Git 提交历史"""
    cmd = ['git', 'log', '--pretty=format:%H|%s|%an|%ad', '--date=short']
    if since:
        cmd.extend(['--since', since])
    
    result = subprocess.run(cmd, capture_output=True, text=True, cwd=r'D:\workspace\first-game')
    
    commits = []
    for line in result.stdout.strip().split('\n'):
        if '|' in line:
            parts = line.split('|')
            if len(parts) >= 4:
                commits.append({
                    'hash': parts[0][:8],
                    'message': parts[1],
                    'author': parts[2],
                    'date': parts[3]
                })
    
    return commits

def parse_conventional_commit(message):
    """解析 Conventional Commit 格式"""
    patterns = {
        'feat': r'^feat(?:\(([^)]+)\))?: (.+)',
        'fix': r'^fix(?:\(([^)]+)\))?: (.+)',
        'docs': r'^docs(?:\(([^)]+)\))?: (.+)',
        'style': r'^style(?:\(([^)]+)\))?: (.+)',
        'refactor': r'^refactor(?:\(([^)]+)\))?: (.+)',
        'perf': r'^perf(?:\(([^)]+)\))?: (.+)',
        'test': r'^test(?:\(([^)]+)\))?: (.+)',
        'chore': r'^chore(?:\(([^)]+)\))?: (.+)',
    }
    
    for type_name, pattern in patterns.items():
        match = re.match(pattern, message)
        if match:
            scope = match.group(1) or ''
            description = match.group(2)
            return type_name, scope, description
    
    return 'other', '', message

def generate_changelog():
    """生成变更日志"""
    commits = get_git_log()
    
    # 按类型分组
    changelog = {
        'feat': [],
        'fix': [],
        'docs': [],
        'refactor': [],
        'perf': [],
        'test': [],
        'chore': [],
        'other': []
    }
    
    for commit in commits:
        type_name, scope, description = parse_conventional_commit(commit['message'])
        entry = {
            'hash': commit['hash'],
            'description': description,
            'scope': scope,
            'author': commit['author'],
            'date': commit['date']
        }
        changelog[type_name].append(entry)
    
    # 生成 Markdown
    md_lines = [
        "# 变更日志",
        "",
        f"生成时间: {datetime.now().strftime('%Y-%m-%d %H:%M')}",
        "",
        "## 格式说明",
        "",
        "本项目使用 [Conventional Commits](https://www.conventionalcommits.org/) 规范。",
        "",
        "- `feat`: 新功能",
        "- `fix`: Bug 修复",
        "- `docs`: 文档更新",
        "- `style`: 代码格式（不影响功能）",
        "- `refactor`: 代码重构",
        "- `perf`: 性能优化",
        "- `test`: 测试相关",
        "- `chore`: 构建/工具变更",
        "",
    ]
    
    type_names = {
        'feat': '✨ 新功能',
        'fix': '🐛 Bug 修复',
        'docs': '📝 文档',
        'refactor': '♻️ 重构',
        'perf': '⚡ 性能优化',
        'test': '✅ 测试',
        'chore': '🔧 维护',
        'other': '📦 其他'
    }
    
    for type_name, entries in changelog.items():
        if entries:
            md_lines.append(f"## {type_names.get(type_name, type_name)}")
            md_lines.append("")
            
            for entry in entries[:20]:  # 每个类型最多显示 20 个
                scope_str = f"**{entry['scope']}**: " if entry['scope'] else ""
                md_lines.append(f"- {scope_str}{entry['description']} ({entry['hash']}) - {entry['author']}, {entry['date']}")
            
            if len(entries) > 20:
                md_lines.append(f"\n*... 还有 {len(entries) - 20} 个提交*")
            
            md_lines.append("")
    
    # 写入文件
    changelog_path = Path(r'D:\workspace\first-game\CHANGELOG.md')
    with open(changelog_path, 'w', encoding='utf-8') as f:
        f.write('\n'.join(md_lines))
    
    print(f"✅ 变更日志已生成: {changelog_path}")
    print(f"总提交数: {len(commits)}")
    
    for type_name, entries in changelog.items():
        if entries:
            print(f"  {type_names.get(type_name, type_name)}: {len(entries)}")

if __name__ == '__main__':
    generate_changelog()

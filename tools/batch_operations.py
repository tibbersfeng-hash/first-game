#!/usr/bin/env python3
"""
批量操作工具集
用于自动化常见的 UE5 编辑器操作
"""

import os
import sys
import subprocess

UE5_CMD = r"D:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
PROJECT = r"D:\workspace\first-game\src_ue5\FirstGame.uproject"

def run_python_script(script_name):
    """运行 UE5 Python 脚本"""
    script_path = os.path.join(r"D:\workspace\first-game\src_ue5\Content\Python", script_name)
    
    cmd = [
        UE5_CMD,
        PROJECT,
        "-stdout",
        "-fullstdoutlogoutput",
        "-nosplash",
        "-unattended",
        "-nullrhi",
        f"-RunPythonScript={script_path}"
    ]
    
    print(f"运行脚本: {script_name}")
    result = subprocess.run(cmd, capture_output=True, text=True, encoding='utf-8', errors='ignore')
    
    if result.returncode == 0:
        print(f"✅ {script_name} 执行成功")
    else:
        print(f"❌ {script_name} 执行失败")
        print(result.stdout)
        print(result.stderr)
    
    return result.returncode == 0

def run_tests():
    """运行所有测试"""
    cmd = [
        UE5_CMD,
        PROJECT,
        "-stdout",
        "-fullstdoutlogoutput",
        "-nosplash",
        "-unattended",
        "-nullrhi",
        "-ExecCmds=Automation RunTests FirstGame; Automation QuitWhenTestsComplete"
    ]
    
    print("运行自动化测试...")
    result = subprocess.run(cmd, capture_output=True, text=True, encoding='utf-8', errors='ignore')
    
    # 解析测试结果
    for line in result.stdout.split('\n'):
        if 'Passed' in line or 'Failed' in line or 'Total' in line:
            print(line)
    
    return 'Failed: 0' in result.stdout

def build_project():
    """编译项目"""
    build_bat = r"D:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat"
    
    cmd = [
        build_bat,
        "FirstGameEditor",
        "Win64",
        "Development",
        f"-Project={PROJECT}",
        "-WaitMutex",
        "-NoLiveCoding"
    ]
    
    print("编译项目...")
    result = subprocess.run(cmd, capture_output=True, text=True, encoding='utf-8', errors='ignore')
    
    if 'Result: Succeeded' in result.stdout:
        print("✅ 编译成功")
        return True
    else:
        print("❌ 编译失败")
        print(result.stdout[-500:])  # 最后 500 字符
        return False

def main():
    """主函数"""
    if len(sys.argv) < 2:
        print("用法: python batch_operations.py <command>")
        print("命令:")
        print("  build          - 编译项目")
        print("  test           - 运行测试")
        print("  run <script>   - 运行 Python 脚本")
        print("  all            - 执行所有操作")
        return
    
    command = sys.argv[1]
    
    if command == 'build':
        success = build_project()
        sys.exit(0 if success else 1)
    
    elif command == 'test':
        success = run_tests()
        sys.exit(0 if success else 1)
    
    elif command == 'run':
        if len(sys.argv) < 3:
            print("请指定脚本名称")
            sys.exit(1)
        script_name = sys.argv[2]
        success = run_python_script(script_name)
        sys.exit(0 if success else 1)
    
    elif command == 'all':
        print("=== 执行所有操作 ===")
        build_project()
        run_tests()
        print("\n完成！")
    
    else:
        print(f"未知命令: {command}")

if __name__ == '__main__':
    main()

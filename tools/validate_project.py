#!/usr/bin/env python3
"""
项目验证脚本
检查项目完整性、编译状态、资产完整性等
"""

import os
import json
import subprocess
from pathlib import Path

PROJECT_ROOT = Path(r"D:\workspace\first-game")
UE5_ROOT = Path(PROJECT_ROOT) / "src_ue5"

class ProjectValidator:
    def __init__(self):
        self.errors = []
        self.warnings = []
        self.info = []
    
    def check_compiled_binaries(self):
        """检查编译产物"""
        print("\n=== 检查编译产物 ===")
        
        binaries = [
            UE5_ROOT / "Binaries/Win64/UnrealEditor-FirstGame.dll",
            UE5_ROOT / "Binaries/Win64/UnrealEditor-FirstGameTests.dll"
        ]
        
        for binary in binaries:
            if binary.exists():
                size_mb = binary.stat().st_size / (1024 * 1024)
                print(f"✅ {binary.name}: {size_mb:.1f} MB")
                self.info.append(f"编译产物: {binary.name}")
            else:
                print(f"❌ {binary.name}: 不存在")
                self.errors.append(f"缺少编译产物: {binary.name}")
    
    def check_source_files(self):
        """检查源文件"""
        print("\n=== 检查源文件 ===")
        
        source_dir = UE5_ROOT / "Source/FirstGame"
        if not source_dir.exists():
            self.errors.append("Source/FirstGame 目录不存在")
            return
        
        cpp_files = list(source_dir.rglob("*.cpp"))
        h_files = list(source_dir.rglob("*.h"))
        
        print(f"C++ 源文件: {len(cpp_files)} 个")
        print(f"C++ 头文件: {len(h_files)} 个")
        
        if len(cpp_files) == 0:
            self.errors.append("未找到 C++ 源文件")
        else:
            self.info.append(f"源文件: {len(cpp_files)} cpp + {len(h_files)} h")
    
    def check_assets(self):
        """检查资产"""
        print("\n=== 检查资产 ===")
        
        content_dir = UE5_ROOT / "Content"
        if not content_dir.exists():
            self.warnings.append("Content 目录不存在")
            return
        
        uasset_files = list(content_dir.rglob("*.uasset"))
        umap_files = list(content_dir.rglob("*.umap"))
        fbx_files = list(content_dir.rglob("*.fbx"))
        
        print(f"UE5 资产: {len(uasset_files)} 个")
        print(f"关卡: {len(umap_files)} 个")
        print(f"FBX 模型: {len(fbx_files)} 个")
        
        # 检查关键资产
        key_assets = [
            "Content/Characters/Huikong/Mesh/SKM_Huikong.uasset",
            "Content/Characters/Huikong/ABP/ABP_Huikong.uasset",
            "Content/Monsters/CandyZombie/Mesh/SK_CandyZombie.uasset",
        ]
        
        for asset in key_assets:
            asset_path = UE5_ROOT / asset
            if asset_path.exists():
                print(f"✅ {Path(asset).name}")
            else:
                print(f"⚠️  {Path(asset).name}: 缺失")
                self.warnings.append(f"关键资产缺失: {asset}")
    
    def check_project_file(self):
        """检查项目文件"""
        print("\n=== 检查项目文件 ===")
        
        uproject = UE5_ROOT / "FirstGame.uproject"
        if not uproject.exists():
            self.errors.append("FirstGame.uproject 不存在")
            return
        
        try:
            with open(uproject, 'r', encoding='utf-8') as f:
                project_data = json.load(f)
            
            print(f"✅ 项目名称: {project_data.get('Description', 'N/A')}")
            print(f"✅ 引擎版本: {project_data.get('EngineAssociation', 'N/A')}")
            
            modules = project_data.get('Modules', [])
            print(f"✅ 模块数量: {len(modules)}")
            
            self.info.append(f"项目: {project_data.get('Description', 'FirstGame')}")
            
        except Exception as e:
            self.errors.append(f"项目文件解析失败: {e}")
    
    def check_documentation(self):
        """检查文档"""
        print("\n=== 检查文档 ===")
        
        docs = [
            PROJECT_ROOT / "CLAUDE.md",
            PROJECT_ROOT / "HANDOVER.md",
            PROJECT_ROOT / "README.md",
        ]
        
        for doc in docs:
            if doc.exists():
                print(f"✅ {doc.name}")
            else:
                print(f"⚠️  {doc.name}: 缺失")
                self.warnings.append(f"文档缺失: {doc.name}")
    
    def generate_report(self):
        """生成验证报告"""
        print("\n" + "="*50)
        print("项目验证报告")
        print("="*50)
        
        if self.errors:
            print(f"\n❌ 错误 ({len(self.errors)}):")
            for error in self.errors:
                print(f"  - {error}")
        
        if self.warnings:
            print(f"\n⚠️  警告 ({len(self.warnings)}):")
            for warning in self.warnings:
                print(f"  - {warning}")
        
        if self.info:
            print(f"\nℹ️  信息 ({len(self.info)}):")
            for info in self.info:
                print(f"  - {info}")
        
        print("\n" + "="*50)
        if not self.errors:
            print("✅ 项目验证通过！")
            return True
        else:
            print("❌ 项目验证失败，请修复上述错误")
            return False

def main():
    validator = ProjectValidator()
    
    print("开始项目验证...\n")
    
    validator.check_compiled_binaries()
    validator.check_source_files()
    validator.check_assets()
    validator.check_project_file()
    validator.check_documentation()
    
    success = validator.generate_report()
    
    # 保存报告到 JSON
    report = {
        'errors': validator.errors,
        'warnings': validator.warnings,
        'info': validator.info,
        'status': 'pass' if success else 'fail'
    }
    
    report_path = PROJECT_ROOT / "docs" / "validation_report.json"
    report_path.parent.mkdir(parents=True, exist_ok=True)
    
    with open(report_path, 'w', encoding='utf-8') as f:
        json.dump(report, f, indent=2, ensure_ascii=False)
    
    print(f"\n报告已保存: {report_path}")
    
    return 0 if success else 1

if __name__ == '__main__':
    exit(main())

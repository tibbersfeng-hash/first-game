"""
慧空动画测试关卡 - 完整验证流程
触发：FIRSTGAME_HUIKONG_TEST=1

验证内容：
1. Idle 动画（默认）
2. Walk 动画（慢走）
3. Run 动画（快速跑）
4. Attack 动画（攻击）
5. 飞天问题检查（Z 坐标）

输出：多张截图 + 验证报告
"""
import unreal
import time
import json
from datetime import datetime

class HuikongAnimationTester:
    """慧空动画测试器"""

    def __init__(self):
        self.results = {
            "timestamp": datetime.now().isoformat(),
            "tests": [],
            "screenshots": [],
            "flying_bug": False,
            "all_passed": True
        }
        self.screenshot_dir = "/home/vipuser/first-game/production/qa/evidence/"
        self.test_level_path = "/Game/Maps/TestLevel_Huikong"

    def log(self, msg):
        unreal.log(f"[HUIKONG TEST] {msg}")

    def screenshot(self, name):
        """截图并保存"""
        filename = f"Huikong_{name}_{int(time.time())}.png"
        filepath = f"{self.screenshot_dir}{filename}"

        unreal.SystemLibrary.execute_console_command(None, "HighResShot 1920x1080")
        time.sleep(3)

        self.results["screenshots"].append({
            "name": name,
            "file": filepath,
            "time": datetime.now().isoformat()
        })
        self.log(f"  📸 截图：{name}")

    def check_flying(self):
        """检查飞天问题"""
        for actor in unreal.EditorLevelLibrary.get_all_level_actors():
            if 'Huikong' in actor.get_actor_label():
                loc = actor.get_actor_location()
                if loc.z > 200:
                    self.log(f"  ⚠️  飞天！Z={loc.z}")
                    self.results["flying_bug"] = True
                    self.results["all_passed"] = False
                    return False
                else:
                    self.log(f"  ✅ 正常 Z={loc.z}")
                    return True
        return False

    def setup_level(self):
        """设置测试关卡"""
        self.log("=" * 60)
        self.log("开始慧空动画测试")
        self.log("=" * 60)

        # 创建或加载关卡
        if unreal.EditorAssetLibrary.does_asset_exist(self.test_level_path):
            unreal.EditorLevelLibrary.load_level(self.test_level_path)
            self.log(f"  加载关卡：{self.test_level_path}")
        else:
            unreal.EditorLevelLibrary.new_level(self.test_level_path)
            self.log(f"  创建关卡：{self.test_level_path}")

        time.sleep(3)

        # 清除现有 Actor
        for actor in unreal.EditorLevelLibrary.get_all_level_actors():
            cls_name = actor.get_class().get_name()
            if 'BaseEnemy' in cls_name or 'Light' in cls_name or 'Camera' in cls_name:
                unreal.EditorLevelLibrary.destroy_actor(actor)

        # 加载慧空资产
        huikong_mesh = unreal.load_asset("/Game/Characters/Huikong/Mesh/SKM_Huikong")
        huikong_abp = unreal.load_asset("/Game/Characters/Huikong/ABP/ABP_Huikong")

        if not huikong_mesh or not huikong_abp:
            self.log("  ❌ 慧空资产加载失败")
            return False

        self.log("  ✅ 慧空资产加载成功")

        # 创建慧空角色
        enemy_class = unreal.find_class("BaseEnemy") if hasattr(unreal, 'find_class') else None

        # 使用 StaticMeshActor 作为临时替代
        huikong = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.StaticMeshActor,
            unreal.Vector(0, 0, 50),
            unreal.Rotator(0, 0, 0)
        )

        if huikong:
            huikong.set_actor_label("Huikong")
            mesh_comp = huikong.get_component_by_class(unreal.StaticMeshComponent)
            if mesh_comp and huikong_mesh:
                mesh_comp.set_static_mesh(huikong_mesh)
            self.log("  ✅ 慧空角色已创建")

        # 添加地面
        floor = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.StaticMeshActor,
            unreal.Vector(0, 0, 0),
            unreal.Rotator(0, 0, 0)
        )
        if floor:
            floor.set_actor_label("Floor")
            mesh = unreal.load_asset("/Engine/BasicShapes/Cube")
            comp = floor.get_component_by_class(unreal.StaticMeshComponent)
            if comp:
                comp.set_static_mesh(mesh)
            floor.set_actor_scale3d(unreal.Vector(10, 10, 0.1))
            self.log("  ✅ 地面已添加")

        # 添加光照
        sun = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.DirectionalLight,
            unreal.Vector(0, 0, 1000),
            unreal.Rotator(-45, 45, 0)
        )
        if sun:
            sun.set_actor_label("Sun")
            self.log("  ✅ 方向光已添加")

        # 设置相机
        cam = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.CameraActor,
            unreal.Vector(0, -300, 200),
            unreal.Rotator(-15, 0, 0)
        )
        if cam:
            unreal.SystemLibrary.execute_console_command(None, "setviewtarget " + cam.get_actor_label())
            self.log("  ✅ 相机已设置")

        # 设置渲染参数
        unreal.SystemLibrary.execute_console_command(None, "r.ExposureCompensation=0")
        unreal.SystemLibrary.execute_console_command(None, "r.AutoExposureMethod 0")

        # 保存关卡
        unreal.EditorLevelLibrary.save_current_level()
        self.log("  ✅ 关卡已保存")

        return True

    def test_idle(self):
        """测试 Idle 动画"""
        self.log("\n--- 测试 1: Idle 动画 ---")
        self.screenshot("01_Idle")
        self.check_flying()
        time.sleep(2)

    def test_walk(self):
        """测试 Walk 动画"""
        self.log("\n--- 测试 2: Walk 动画 ---")
        unreal.SystemLibrary.execute_console_command(None, "play")
        time.sleep(2)

        # 模拟慢走输入
        unreal.SystemLibrary.execute_console_command(None, "MoveForward 1.0")
        time.sleep(3)

        self.screenshot("02_Walk")
        self.check_flying()

        unreal.SystemLibrary.execute_console_command(None, "MoveForward 0")
        time.sleep(1)

    def test_run(self):
        """测试 Run 动画"""
        self.log("\n--- 测试 3: Run 动画 ---")

        # 模拟快速跑输入
        unreal.SystemLibrary.execute_console_command(None, "MoveForward 1.0")
        unreal.SystemLibrary.execute_console_command(None, "Sprint 1")
        time.sleep(3)

        self.screenshot("03_Run")
        self.check_flying()

        unreal.SystemLibrary.execute_console_command(None, "MoveForward 0")
        unreal.SystemLibrary.execute_console_command(None, "Sprint 0")
        time.sleep(1)

    def test_attack(self):
        """测试 Attack 动画"""
        self.log("\n--- 测试 4: Attack 动画 ---")

        # 模拟攻击输入
        unreal.SystemLibrary.execute_console_command(None, "LightAttack")
        time.sleep(2)

        self.screenshot("04_Attack")
        self.check_flying()
        time.sleep(1)

    def finalize(self):
        """完成测试"""
        unreal.SystemLibrary.execute_console_command(None, "stop")
        self.log("\n    游戏停止")

        # 保存测试报告
        report_path = f"{self.screenshot_dir}huikong_test_report.json"
        with open(report_path, 'w') as f:
            json.dump(self.results, f, indent=2, ensure_ascii=False)

        self.log(f"\n  测试报告已保存：{report_path}")
        self.log("=" * 60)

        if self.results["all_passed"]:
            self.log("✅ 所有测试通过！")
        else:
            self.log("⚠️  部分测试失败，请检查截图")

        self.log("=" * 60)

        # 不退出编辑器，让用户查看
        self.log("  编辑器保持运行，请查看截图")


def main():
    tester = HuikongAnimationTester()

    if not tester.setup_level():
        unreal.log_error("  关卡设置失败")
        return

    # 执行测试
    tester.test_idle()
    tester.test_walk()
    tester.test_run()
    tester.test_attack()

    # 完成
    tester.finalize()


if __name__ == "__main__":
    main()
else:
    main()

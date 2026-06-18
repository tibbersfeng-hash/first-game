"""
UE5 Editor Python 脚本 — 一键创建第一个可玩关卡

使用方法:
  方式 A: 在 UE5 Editor 的 Python 控制台粘贴执行
    exec(open(r'/path/to/create_first_level.py').read())

  方式 B: 命令行启动时自动执行
    UnrealEditor-Cmd <project> -ExecutePythonScript="/path/to/create_first_level.py"

  方式 C: 在 UE5 Output Log 中
    切换到 Python 模式 → 粘贴上述 exec(...) 行

这个脚本做的事:
  1. 创建新关卡 /Game/Maps/FirstCombatMap
  2. 在关卡中放置 1 个 LevelBuilder Actor
  3. 配置 LevelBuilder: 3 个房间、使用 BaseEnemy、启用 Debug
  4. 保存关卡

运行后可直接 PIE 进入可玩关卡。
"""

import unreal

# ─── 配置 ──────────────────────────────────────────────────────────────
MAP_PATH = "/Game/Maps/FirstCombatMap"
LEVEL_BUILDER_CLASS = "/Script/FirstGame.LevelBuilder"  # C++ 模块名是 FirstGame
TOTAL_ROOMS = 3
BASE_ENEMY_COUNT = 3
DEBUG_MODE = True

# ─── Step 1: 创建新关卡 ──────────────────────────────────────────────
unreal.log("=== 创建第一个可玩关卡 ===")

# 如果已存在，先打开它
if unreal.EditorAssetLibrary.does_asset_exist(MAP_PATH):
    unreal.log(f"关卡已存在: {MAP_PATH}, 直接打开")
    unreal.EditorLevelLibrary.load_level(MAP_PATH)
else:
    unreal.log(f"创建新关卡: {MAP_PATH}")
    # 确保目录存在
    map_dir = "/".join(MAP_PATH.split("/")[:-1])
    if not unreal.EditorAssetLibrary.does_directory_exist(map_dir):
        unreal.EditorAssetLibrary.make_directory(map_dir)
    unreal.EditorLevelLibrary.new_level_from_template(MAP_PATH, "")

# ─── Step 2: 放置 LevelBuilder Actor ──────────────────────────────────
unreal.log("Step 2: 放置 LevelBuilder Actor")

# 加载 LevelBuilder Class
LevelBuilderClass = unreal.load_class(None, LEVEL_BUILDER_CLASS)
if not LevelBuilderClass:
    unreal.log_error(f"无法加载 LevelBuilder Class: {LEVEL_BUILDER_CLASS}")
    unreal.log_error("请确保 C++ 模块已编译, 且模块名为 FirstGame")
else:
    # 在关卡原点 Spawn
    builder = unreal.EditorLevelLibrary.spawn_actor_from_class(
        LevelBuilderClass,
        unreal.Vector(0, 0, 0),
        unreal.Rotator(0, 0, 0)
    )

    if not builder:
        unreal.log_error("Spawn LevelBuilder 失败")
    else:
        unreal.log(f"LevelBuilder Spawn 成功: {builder.get_name()}")

        # ─── Step 3: 配置 LevelBuilder 属性 ─────────────────────────
        unreal.log("Step 3: 配置 LevelBuilder 属性")

        # 设置 TotalRooms
        builder.set_editor_property("total_rooms", TOTAL_ROOMS)
        unreal.log(f"  TotalRooms = {TOTAL_ROOMS}")

        # 设置 BaseEnemyCountPerRoom
        builder.set_editor_property("base_enemy_count_per_room", BASE_ENEMY_COUNT)
        unreal.log(f"  BaseEnemyCountPerRoom = {BASE_ENEMY_COUNT}")

        # 设置 Debug 模式
        builder.set_editor_property("debug_mode", DEBUG_MODE)
        unreal.log(f"  DebugMode = {DEBUG_MODE}")

        # 尝试加载 BaseEnemy Class 并设置
        try:
            BaseEnemyClass = unreal.load_class(None, "/Script/FirstGame.BaseEnemy")
            if BaseEnemyClass:
                # 注意: TSubclassOf 在 Python 里的设置方式
                # 由于 BaseEnemy 是抽象基类, 需要具体子类
                # 这里先设置为 BaseEnemy, 用户后续可在编辑器里替换
                builder.set_editor_property("default_enemy_class", BaseEnemyClass)
                unreal.log(f"  DefaultEnemyClass = {BaseEnemyClass.get_name()}")
        except Exception as e:
            unreal.log_warning(f"  无法设置 DefaultEnemyClass: {e}")
            unreal.log_warning(f"  请在 Details 面板手动设置敌人 Class")

        # ─── Step 4: 保存关卡 ───────────────────────────────────────
        unreal.log("Step 4: 保存关卡")
        unreal.EditorLevelLibrary.save_current_level()
        unreal.log(f"关卡已保存: {MAP_PATH}")

        # ─── 完成 ──────────────────────────────────────────────────
        unreal.log("=== 关卡创建完成 ===")
        unreal.log("操作提示:")
        unreal.log("  1. 按 Ctrl+Alt+P 或点 Play 进入 PIE")
        unreal.log("  2. 观察 Output Log 里的 [LevelBuilder] 日志")
        unreal.log("  3. 如果看不到角色, 在 Details 面板调整 LevelBuilder 的")
        unreal.log("     HUDWidgetClass 属性, 指向 HUDWidget Blueprint")

        # ─── 选中 Actor 便于用户查看 ────────────────────────────────
        unreal.EditorLevelLibrary.set_selected_level_actors([builder])

# ─── 额外提示 ──────────────────────────────────────────────────────────
unreal.log("=== 提示 ===")
unreal.log("如果想调整关卡配置, 可以:")
unreal.log("  A. 选中 LevelBuilder Actor, 在 Details 面板修改属性")
unreal.log("  B. 修改后重新 PIE 即可看到变化 (BeginPlay 会重新构建)")
unreal.log("  C. 如果想永久配置, 右键 LevelBuilder → Create Blueprint based on...")
unreal.log("     然后修改 Blueprint 的 Class Defaults")
unreal.log("  D. 在 GameMode Blueprint 的 Details 面板设置 LevelBuilderClass")
unreal.log("     为该 Blueprint 子类, 这样每次新关卡都自动使用你的配置")

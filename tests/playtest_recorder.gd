extends Node2D

## ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
## 格斗萌主 — 自动战斗录像 + 截图测试
## 在 Xvfb 虚拟显示下运行，自动执行战斗并截图
## ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

var _player: CharacterBody2D = null
var _enemy: CharacterBody2D = null
var _frame := 0
var _phase := 0  # 0=init,1=walk,2=attack,3=kill,4=done
var _phase_frame := 0
var _enemy_dead := false
var _enemy_init_hp := 0
var _total_dmg := 0
var _hits := 0
var _combo_log: Array[String] = []
var _hp_snap: Array[Dictionary] = []
var _screenshots: Array[Dictionary] = []
var _capture := false

func _ready() -> void:
	# 连接信号
	SignalBus.hit_confirmed.connect(_on_hit)
	SignalBus.combo_updated.connect(_on_combo)
	SignalBus.combo_dropped.connect(func(): _combo_log.append("中断@F%d" % _frame))
	SignalBus.enemy_died.connect(func(e): _enemy_dead = true)

	# 检测是否在虚拟显示环境
	_capture = OS.get_environment("RECORD_GAME") == "1"

	# 加载战斗场景
	var scene = load("res://src/gameplay/scenes/combat_test.tscn")
	if not scene:
		printraw("❌ 场景加载失败\n")
		get_tree().quit()
		return

	var combat = scene.instantiate()
	add_child(combat)

	_player = combat.get_node("Player")
	_enemy = combat.get_node("Enemy")
	_player.position = Vector2(150, 200)
	_player.facing = 1

	printraw("✅ 战斗场景加载 — Player@(%d,%d) Enemy@(%d,%d)\n" % [
		int(_player.position.x), int(_player.position.y),
		int(_enemy.position.x), int(_enemy.position.y)])


func _physics_process(delta: float) -> void:
	_frame += 1
	_phase_frame += 1

	# 第1帧初始化
	if _frame == 1:
		_enemy_init_hp = _enemy.health.current_hp
		printraw("   敌人HP:%d 玩家能量:%d\n" % [_enemy_init_hp, _player.energy.current_energy])
		printraw("⏱️ 自动战斗开始 (录制=%s)\n" % str(_capture))
		_screenshot("start")
		_phase = 1
		_phase_frame = 0
		return

	# HP快照
	if _frame % 20 == 0:
		_hp_snap.append({"f": _frame, "hp": _enemy.health.current_hp})

	match _phase:
		1: _phase_walk()
		2: _phase_combo()
		3: _phase_kill()
		4: _phase_done()

	# 状态报告 (每60帧)
	if _frame % 60 == 0:
		printraw("[F%d] Player=%s Enemy=%s HP=%d/%d\n" % [
			_frame,
			_player.state_machine.get_current_state_name(),
			_enemy.state_machine.get_current_state_name(),
			_enemy.health.current_hp, _enemy_init_hp])

	if _frame > 900:
		printraw("⏰ 超时\n")
		_phase = 4


func _phase_walk() -> void:
	if _phase_frame == 1:
		printraw("\n🏃 [F%d] 玩家向右移动...\n" % _frame)
	_player.movement.apply_movement(_player, 1.0, false)
	_player.movement.apply_gravity(_player, get_physics_process_delta_time())
	_player.facing = 1
	_player.visual_pivot.scale.x = 1

	if _phase_frame == 15:
		_screenshot("walk_mid")
	if _phase_frame >= 30:
		_player.movement.apply_movement(_player, 0.0, false)
		printraw("   位置:(%d,%d) 地面:%s\n" % [
			int(_player.position.x), int(_player.position.y),
			"是" if _player.is_on_floor() else "否"])
		_screenshot("walk_end")
		_next_phase()


func _phase_combo() -> void:
	# 注入攻击输入
	match _phase_frame:
		1:
			printraw("\n⚔️ [F%d] 轻攻击连段 J-J-J\n" % _frame)
			_inject("attack_light")
		15:
			_inject("attack_light")
		30:
			_inject("attack_light")
		50:
			printraw("   [F%d] 基础连段 J-J-K-L\n" % _frame)
			_inject("attack_light")
		65:
			_inject("attack_light")
		80:
			_inject("attack_heavy")
			printraw("   [F%d] 重攻击!\n" % _frame)
		95:
			_inject("attack_special")
			printraw("   [F%d] 特殊攻击!\n" % _frame)

	# 攻击中的截图
	if _phase_frame in [5, 20, 55, 85]:
		_screenshot("attack_f%d_p%d" % [_frame, _phase_frame])

	if _phase_frame >= 120:
		printraw("   敌人HP:%d/%d\n" % [_enemy.health.current_hp, _enemy_init_hp])
		_screenshot("combo_end")
		_next_phase()


func _phase_kill() -> void:
	if _phase_frame == 1:
		printraw("\n💀 [F%d] 连续攻击击杀...\n" % _frame)

	if _phase_frame % 12 == 1:
		_inject("attack_light")

	if _enemy_dead:
		printraw("   [F%d] 敌人被击杀!\n" % _frame)
		_screenshot("enemy_dead")
		# 等一小段看死亡动画
		if _phase_frame > 30:
			_next_phase()

	if _phase_frame > 400:
		printraw("   ⚠️ HP=%d\n" % _enemy.health.current_hp)
		_next_phase()


func _phase_done() -> void:
	if _phase != 4:
		return
	_phase = 4  # prevent re-entry
	_screenshot("final")
	_report()
	get_tree().quit()


func _next_phase() -> void:
	_phase += 1
	_phase_frame = 0

func _inject(action: String) -> void:
	var pri = {"attack_light": 40, "attack_heavy": 50, "attack_special": 60}
	_player.get_node("InputBuffer")._record_action(action, pri[action])

func _on_hit(target: Node2D, damage: int, damage_type: int, is_crit: bool) -> void:
	var names = ["轻", "重", "特"]
	_total_dmg += damage
	_hits += 1
	printraw("   🎯 %s攻击→%d伤害%s HP=%d\n" % [
		names[damage_type] if damage_type < 3 else "?",
		damage, " 💥暴击!" if is_crit else "",
		_enemy.health.current_hp])

func _on_combo(count: int, name: String) -> void:
	_combo_log.append("%d段@F%d[%s]" % [count, _frame, name])
	printraw("   🔗 连段:%d段(%s)\n" % [count, name])

func _screenshot(tag: String) -> void:
	# 记录截图标记（实际截图由外部脚本通过 xwd/ffmpeg 完成）
	_screenshots.append({"frame": _frame, "tag": tag})
	printraw("   📸 screenshot:%s @ F%d\n" % [tag, _frame])


func _report() -> void:
	printraw("\n" + "━".repeat(50) + "\n")
	printraw("📊 战斗结果报告\n")
	printraw("━".repeat(50) + "\n")

	printraw("\n🎮 游戏运行:\n")
	_v("场景加载", _player != null)
	_v("角色可移动", _player.position.x > 150)
	_v("角色在地面", _player.is_on_floor())

	printraw("\n⚔️ 战斗:\n")
	printraw("  命中: %d\n" % _hits)
	printraw("  总伤害: %d\n" % _total_dmg)
	if _hits > 0:
		printraw("  平均: %.1f\n" % (float(_total_dmg) / _hits))
	_v("有命中", _hits > 0)
	_v("有伤害", _total_dmg > 0)

	printraw("\n👹 敌人:\n")
	if _enemy:
		var hp = _enemy.health.current_hp
		printraw("  HP: %d→%d (%.0f%%)\n" % [_enemy_init_hp, hp, (1.0-float(hp)/maxi(_enemy_init_hp,1))*100])
		printraw("  状态: %s\n" % _enemy.state_machine.get_current_state_name())
	_v("敌人受伤", _enemy.health.current_hp < _enemy_init_hp)
	_v("击杀敌人", _enemy_dead)

	printraw("\n🔗 连段: %d事件\n" % _combo_log.size())
	for c in _combo_log:
		printraw("  · %s\n" % c)
	_v("连段工作", _combo_log.size() > 0)

	printraw("\n📈 HP:\n")
	for h in _hp_snap:
		var pct = float(h.hp) / maxi(_enemy_init_hp, 1)
		var n = int(pct * 25)
		printraw("  F%4d [%s%s] %3d\n" % [h.f, "█".repeat(maxi(n,0)), "░".repeat(maxi(25-n,0)), h.hp])

	printraw("\n📸 截图标记: %d\n" % _screenshots.size())
	for s in _screenshots:
		printraw("  F%d → %s\n" % [s.frame, s.tag])

	printraw("\n" + "━".repeat(50) + "\n")
	var ok = _hits > 0 and _total_dmg > 0 and _enemy_dead
	printraw("%s\n" % ("🎉 完整验证通过!" if ok else "⚠️ 部分未通过"))


func _v(desc: String, ok: bool) -> void:
	printraw("  %s %s\n" % ["✅" if ok else "❌", desc])

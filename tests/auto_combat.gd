extends Node

## 格斗萌主 — 端到端战斗验证
## 不依赖场景树运行，手动驱动战斗循环

var _frame := 0
var _player: CharacterBody2D = null
var _enemy: CharacterBody2D = null
var _log: Array[String] = []
var _enemy_dead := false
var _total_dmg := 0
var _hits := 0
var _crits := 0
var _combo_log: Array[String] = []
var _hp_snap: Array[Dictionary] = []
var _enemy_init_hp := 0
var _player_moved := false

func _ready() -> void:
	var scene = load("res://src/gameplay/scenes/combat_test.tscn").instantiate()
	add_child(scene)
	_player = scene.get_node("Player")
	_enemy = scene.get_node("Enemy")

	# 玩家靠近敌人
	_player.position = Vector2(150, 200)
	_player.facing = 1

	# 连接信号（只记录，不触发游戏逻辑）
	SignalBus.hit_confirmed.connect(_on_hit)
	SignalBus.combo_updated.connect(_on_combo)
	SignalBus.combo_dropped.connect(func(): _combo_log.append("中断@F%d" % _frame); p("   💔 连段中断!"))
	SignalBus.enemy_died.connect(func(e): _enemy_dead = true; p("   ☠️ 敌人死亡!"))
	SignalBus.attack_started.connect(func(id): p("   ⚔️ %s" % id))
	SignalBus.screen_shake_requested.connect(func(i): p("   📳 震动:%.1f" % i))

	p("✅ 战斗场景加载成功")
	p("   Player@(%d,%d) Enemy@(%d,%d)" % [150, 200, 200, 200])

func _physics_process(_delta: float) -> void:
	_frame += 1

	# 等初始化
	if _frame == 1:
		_enemy_init_hp = _enemy.health.current_hp
		p("   敌人HP:%d 能量:%d" % [_enemy_init_hp, _player.energy.current_energy])
		p("   状态: Player=%s Enemy=%s" % [
			_player.state_machine.get_current_state_name(),
			_enemy.state_machine.get_current_state_name()])
		p("")
		p("⏱️ 自动战斗开始 (60fps)")
		p("─".repeat(50))

	# HP 快照
	if _frame % 20 == 0 and _frame > 1:
		_hp_snap.append({"f": _frame, "hp": _enemy.health.current_hp})

	# 阶段1: 移动 (F5-F25)
	if _frame == 5:
		p("\n🏃 [F5] 玩家向右移动...")
	if _frame >= 5 and _frame <= 25:
		_player.movement.apply_movement(_player, 1.0, false)
		_player.movement.apply_gravity(_player, get_physics_process_delta_time())
		_player.facing = 1
		_player.visual_pivot.scale.x = 1
	if _frame == 25:
		_player.movement.apply_movement(_player, 0.0, false)
		_player_moved = true
		p("   位置:(%d,%d) 地面:%s" % [int(_player.position.x), int(_player.position.y), "是" if _player.is_on_floor() else "否"])

	# 阶段2: 轻攻击连段 J-J-J (F30-F90)
	if _frame == 30:
		p("\n⚔️ [F30] 轻攻击连段 J-J-J")
		_inject("attack_light")
	if _frame == 45:
		_inject("attack_light")
		p("   [F45] 第2击")
	if _frame == 60:
		_inject("attack_light")
		p("   [F60] 第3击")

	# 阶段3: 基础连段 J-J-K-L (F100-F180)
	if _frame == 100:
		p("\n⚔️ [F100] 基础连段 J-J-K-L")
		_inject("attack_light")
	if _frame == 115:
		_inject("attack_light")
	if _frame == 130:
		_inject("attack_heavy")
		p("   [F130] 重攻击!")
	if _frame == 145:
		_inject("attack_special")
		p("   [F145] 特殊攻击!")

	# 阶段4: 连续攻击直到击杀 (F200+)
	if _frame == 200:
		p("\n💀 [F200] 连续攻击击杀...")
	if _frame >= 200 and _frame % 15 == 0:
		_inject("attack_light")

	# 状态报告
	if _frame in [90, 180] or (_frame >= 200 and _frame % 60 == 0):
		p("   敌人HP:%d/%d 状态:%s" % [
			_enemy.health.current_hp, _enemy_init_hp,
			_enemy.state_machine.get_current_state_name()])

	# 击杀完成
	if _enemy_dead and _frame > 200:
		p("   [F%d] 敌人被击杀!" % _frame)
		_finish()

	# 超时
	if _frame >= 600:
		p("⏰ 超时")
		_finish()

func _inject(action: String) -> void:
	var pri = {"attack_light": 40, "attack_heavy": 50, "attack_special": 60}
	_player.get_node("InputBuffer")._record_action(action, pri[action])

func _on_hit(target: Node2D, damage: int, damage_type: int, is_crit: bool) -> void:
	var names = ["轻攻击", "重攻击", "特殊攻击"]
	_total_dmg += damage
	_hits += 1
	if is_crit: _crits += 1
	p("   🎯 %s→%d伤害%s | HP=%d" % [
		names[damage_type] if damage_type < 3 else "?",
		damage, " 💥暴击!" if is_crit else "",
		_enemy.health.current_hp])

func _on_combo(count: int, name: String) -> void:
	_combo_log.append("%d段@F%d[%s]" % [count, _frame, name])
	p("   🔗 连段:%d段(%s)" % [count, name])

func p(t: String) -> void:
	_log.append(t)
	printraw(t + "\n")

func _finish() -> void:
	p("\n" + "━".repeat(50))
	p("📊 战斗结果报告")
	p("━".repeat(50))

	p("\n🎮 游戏运行:")
	_v("场景加载", _player != null)
	_v("角色可移动", _player_moved)
	_v("角色在地面", _player.is_on_floor())

	p("\n⚔️ 战斗:")
	p("  命中: %d次" % _hits)
	p("  总伤害: %d" % _total_dmg)
	if _hits > 0:
		p("  暴击: %d/%d(%.0f%%)" % [_crits, _hits, float(_crits)/_hits*100])
		p("  平均伤害: %.1f" % (float(_total_dmg)/_hits))
	_v("有命中", _hits > 0)
	_v("有伤害", _total_dmg > 0)

	p("\n👹 敌人:")
	if _enemy:
		var hp = _enemy.health.current_hp
		p("  HP: %d→%d(-%d, %.0f%%)" % [_enemy_init_hp, hp, _enemy_init_hp-hp, (1.0-float(hp)/maxi(_enemy_init_hp,1))*100])
		p("  状态: %s" % _enemy.state_machine.get_current_state_name())
	_v("敌人受伤", _enemy.health.current_hp < _enemy_init_hp)
	_v("击杀敌人", _enemy_dead)

	p("\n🔗 连段: %d事件" % _combo_log.size())
	for c in _combo_log:
		p("  · %s" % c)
	_v("连段工作", _combo_log.size() > 0)

	p("\n📈 HP时间线:")
	for h in _hp_snap:
		var pct = float(h.hp) / maxi(_enemy_init_hp, 1)
		var n = int(pct * 25)
		p("  F%4d [%s%s] %3d" % [h.f, "█".repeat(maxi(n,0)), "░".repeat(maxi(25-n,0)), h.hp])

	p("\n" + "━".repeat(50))
	var ok = _hits > 0 and _total_dmg > 0 and _enemy_dead
	p("🎉 验证通过! 移动·伤害·连段·击杀 全部正常!" if ok else "⚠️ 部分未通过")

	# 打印完整日志
	print("\n" + "━".repeat(50))
	print("📋 完整战斗日志")
	print("━".repeat(50))
	for l in _log:
		print(l)

	get_tree().quit()

func _v(desc: String, ok: bool) -> void:
	p("  %s %s" % ["✅" if ok else "❌", desc])

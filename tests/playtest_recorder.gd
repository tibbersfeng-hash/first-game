extends Node2D

## 格斗萌主 — 自动战斗 + Godot 内置截图
## 使用 get_viewport().get_texture() 截图，不依赖外部工具

var _player: CharacterBody2D = null
var _enemy: CharacterBody2D = null
var _frame := 0
var _phase := 0
var _phase_frame := 0
var _enemy_dead := false
var _enemy_init_hp := 0
var _total_dmg := 0
var _hits := 0
var _combo_log: Array[String] = []
var _hp_snap: Array[Dictionary] = []
var _ss_dir := ""
var _next_ss := 10
var _ss_interval := 10

func _ready() -> void:
	_ss_dir = OS.get_environment("SCREENSHOT_DIR")
	if _ss_dir == "":
		_ss_dir = "/tmp/first-game-screenshots"
	DirAccess.make_dir_recursive_absolute(_ss_dir)

	SignalBus.hit_confirmed.connect(_on_hit)
	SignalBus.combo_updated.connect(_on_combo)
	SignalBus.combo_dropped.connect(func(): _combo_log.append("中断@F%d" % _frame))
	SignalBus.enemy_died.connect(func(e): _enemy_dead = true)

	var scene = load("res://src/gameplay/scenes/combat_test.tscn")
	if not scene:
		printraw("❌ 场景加载失败\n"); get_tree().quit(); return

	var combat = scene.instantiate()
	add_child(combat)
	_player = combat.get_node("Player")
	_enemy = combat.get_node("Enemy")
	_player.position = Vector2(150, 200)
	_player.facing = 1

	printraw("✅ 场景加载 截图目录: %s\n" % _ss_dir)


func _physics_process(delta: float) -> void:
	_frame += 1; _phase_frame += 1

	if _frame == 1:
		_enemy_init_hp = _enemy.health.current_hp
		printraw("   敌人HP:%d 能量:%d\n" % [_enemy_init_hp, _player.energy.current_energy])
		printraw("⏱️ 战斗开始\n")
		_ss("0001_start"); _phase = 1; _phase_frame = 0; return

	if _frame == _next_ss:
		_ss("%04d" % _frame); _next_ss += _ss_interval
	if _frame % 20 == 0:
		_hp_snap.append({"f": _frame, "hp": _enemy.health.current_hp})

	match _phase:
		1: _ph_walk()
		2: _ph_combo()
		3: _ph_kill()
		4: _ph_done()

	if _frame % 60 == 0:
		printraw("[F%d] P=%s E=%s HP=%d/%d\n" % [_frame, _player.state_machine.get_current_state_name(), _enemy.state_machine.get_current_state_name(), _enemy.health.current_hp, _enemy_init_hp])
	if _frame > 900:
		_phase = 4


func _ph_walk() -> void:
	if _phase_frame == 1: printraw("\n🏃 [F%d] 移动\n" % _frame)
	_player.movement.apply_movement(_player, 1.0, false)
	_player.movement.apply_gravity(_player, get_physics_process_delta_time())
	_player.facing = 1; _player.visual_pivot.scale.x = 1
	if _phase_frame >= 30:
		_player.movement.apply_movement(_player, 0.0, false)
		printraw("   位置:(%d,%d) 地面:%s\n" % [int(_player.position.x), int(_player.position.y), "是" if _player.is_on_floor() else "否"])
		_ss("%04d_walk" % _frame); _np()

func _ph_combo() -> void:
	match _phase_frame:
		1: printraw("\n⚔️ [F%d] J-J-J\n" % _frame); _inject("attack_light")
		15: _inject("attack_light")
		30: _inject("attack_light")
		50: printraw("⚔️ [F%d] J-J-K-L\n" % _frame); _inject("attack_light")
		65: _inject("attack_light")
		80: _inject("attack_heavy"); printraw("   重!\n")
		95: _inject("attack_special"); printraw("   特!\n")
	if _phase_frame in [10, 55, 90]: _ss("%04d_combo" % _frame)
	if _phase_frame >= 120: _ss("%04d_combo_end" % _frame); _np()

func _ph_kill() -> void:
	if _phase_frame == 1: printraw("\n💀 [F%d] 击杀\n" % _frame)
	if _phase_frame % 12 == 1: _inject("attack_light")
	if _enemy_dead:
		printraw("   ☠️ 击杀!\n"); _ss("%04d_dead" % _frame)
		if _phase_frame > 20: _np()
	if _phase_frame > 400: _np()

func _ph_done() -> void:
	if _phase != 4: return
	_phase = 4; _ss("%04d_final" % _frame); _report()
	printraw("\n💡 GIF: ffmpeg -framerate 6 -i %s/shot_%%s.png %s/combat.gif\n" % [_ss_dir, _ss_dir])
	get_tree().quit()


func _np() -> void: _phase += 1; _phase_frame = 0
func _inject(a: String) -> void: _player.get_node("InputBuffer")._record_action(a, {"attack_light": 40, "attack_heavy": 50, "attack_special": 60}[a])

func _on_hit(t: Node2D, d: int, dt: int, c: bool) -> void:
	var n = ["轻","重","特"]; _total_dmg += d; _hits += 1
	printraw("   🎯 %s→%d%s HP=%d\n" % [n[dt] if dt < 3 else "?", d, " 💥暴击!" if c else "", _enemy.health.current_hp])

func _on_combo(c: int, nm: String) -> void:
	_combo_log.append("%d段@F%d[%s]" % [c, _frame, nm]); printraw("   🔗 %d段(%s)\n" % [c, nm])

func _ss(tag: String) -> void:
	var img = get_viewport().get_texture().get_image()
	if img:
		var p = "%s/shot_%s.png" % [_ss_dir, tag]
		img.save_png(p); printraw("   📸 %s\n" % p)

func _report() -> void:
	printraw("\n" + "━".repeat(50) + "\n📊 结果\n" + "━".repeat(50) + "\n")
	printraw("\n🎮:\n"); _v("场景", _player != null); _v("移动", _player.position.x > 150); _v("地面", _player.is_on_floor())
	printraw("\n⚔️: 命中:%d 伤害:%d\n" % [_hits, _total_dmg])
	if _hits > 0: printraw("  平均:%.1f\n" % (float(_total_dmg)/_hits))
	_v("命中", _hits > 0); _v("伤害", _total_dmg > 0)
	printraw("\n👹:\n")
	if _enemy: printraw("  HP:%d→%d %s\n" % [_enemy_init_hp, _enemy.health.current_hp, _enemy.state_machine.get_current_state_name()])
	_v("受伤", _enemy.health.current_hp < _enemy_init_hp); _v("击杀", _enemy_dead)
	printraw("\n🔗: %d\n" % _combo_log.size())
	for c in _combo_log: printraw("  · %s\n" % c)
	_v("连段", _combo_log.size() > 0)
	printraw("\n📈:\n")
	for h in _hp_snap:
		var pct = float(h.hp) / maxi(_enemy_init_hp, 1); var n = int(pct * 25)
		printraw("  F%4d [%s%s] %3d\n" % [h.f, "█".repeat(maxi(n,0)), "░".repeat(maxi(25-n,0)), h.hp])
	printraw("\n" + "━".repeat(50) + "\n")
	printraw("%s\n" % ("🎉 通过!" if _hits > 0 and _total_dmg > 0 and _enemy_dead else "⚠️ 未通过"))

func _v(d: String, ok: bool) -> void: printraw("  %s %s\n" % ["✅" if ok else "❌", d])

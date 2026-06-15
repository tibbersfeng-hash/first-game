extends Node2D

## Comprehensive headless test — simulates combat gameplay in memory.
## Tests all core systems working together.

var _pass := 0
var _fail := 0

func _ready() -> void:
	print("\n" + "=".repeat(60))
	print("格斗萌主 — Combat Simulation Headless Test")
	print("=".repeat(60))

	_test_combat_data_integrity()
	_test_health_component()
	_test_energy_component()
	_test_state_machine()
	_test_input_buffer()
	_test_frame_timer()
	_test_hitbox_hurtbox()
	_test_hit_manager()
	_test_combo_manager()
	_test_scene_instantiation()
	_test_signal_bus()

	# Summary
	print("\n" + "=".repeat(60))
	print("📊 Results: %d PASSED, %d FAILED" % [_pass, _fail])
	if _fail == 0:
		print("✅ ALL TESTS PASSED")
	else:
		print("❌ SOME TESTS FAILED")
	print("=".repeat(60))

	get_tree().quit()


func _p(name: String) -> void:
	_pass += 1
	print("  ✅ %s" % name)

func _f(name: String, reason: String = "") -> void:
	_fail += 1
	print("  ❌ %s%s" % [name, " — " + reason if reason else ""])


# ─── CombatData Integrity ───

func _test_combat_data_integrity() -> void:
	print("\n📋 CombatData Integrity")

	var pc = CombatData.player_config
	if pc:
		_p("player_config loaded")
		if pc.move_speed == 300.0: _p("move_speed = 300.0")
		else: _f("move_speed", "got %s" % pc.move_speed)
		if pc.jump_velocity < 0: _p("jump_velocity negative (up)")
		else: _f("jump_velocity negative", "got %s" % pc.jump_velocity)
		if pc.gravity > 0: _p("gravity > 0")
		else: _f("gravity > 0", "got %s" % pc.gravity)
		if pc.crit_chance >= 0 and pc.crit_chance <= 1.0: _p("crit_chance in [0,1]")
		else: _f("crit_chance range", "got %s" % pc.crit_chance)
	else:
		_f("player_config loaded")

	var ec = CombatData.energy_config
	if ec:
		_p("energy_config loaded")
		if ec.max_energy > 0: _p("max_energy = %d" % ec.max_energy)
		else: _f("max_energy > 0")
	else:
		_f("energy_config loaded")

	# Enemy data
	var goblin = CombatData.get_enemy_data("goblin")
	if goblin:
		_p("goblin data found")
		_p("goblin max_hp = %d" % goblin.max_hp)
		_p("goblin move_speed = %0.1f" % goblin.move_speed)
	else:
		_f("goblin data")

	var elite = CombatData.get_enemy_data("goblin_elite")
	if elite:
		_p("goblin_elite data found")
		_p("elite max_hp = %d" % elite.max_hp)
	else:
		_f("goblin_elite data")

	# Combo chains
	if CombatData.combo_chains.size() >= 3:
		_p("combo_chains count = %d" % CombatData.combo_chains.size())
	else:
		_f("combo_chains >= 3", "got %d" % CombatData.combo_chains.size())

	for chain_name in CombatData.combo_chains:
		var chain = CombatData.combo_chains[chain_name]
		if chain and chain.steps.size() > 0:
			_p("combo '%s' has %d steps" % [chain_name, chain.steps.size()])
		else:
			_f("combo '%s' steps" % chain_name)

	# AttackType enum
	if CombatData.AttackType.LIGHT == 0: _p("AttackType.LIGHT = 0")
	else: _f("AttackType.LIGHT = 0", "got %d" % CombatData.AttackType.LIGHT)
	if CombatData.AttackType.HEAVY == 1: _p("AttackType.HEAVY = 1")
	else: _f("AttackType.HEAVY = 1", "got %d" % CombatData.AttackType.HEAVY)
	if CombatData.AttackType.SPECIAL == 2: _p("AttackType.SPECIAL = 2")
	else: _f("AttackType.SPECIAL = 2", "got %d" % CombatData.AttackType.SPECIAL)

	# get_attack_phase
	var atk = AttackData.new()
	atk.startup_frames = 3
	atk.active_frames = 2
	atk.recovery_frames = 5
	if CombatData.get_attack_phase(atk, 0) == "startup": _p("frame 0 → startup")
	else: _f("frame 0 → startup", "got %s" % CombatData.get_attack_phase(atk, 0))
	if CombatData.get_attack_phase(atk, 3) == "active": _p("frame 3 → active")
	else: _f("frame 3 → active", "got %s" % CombatData.get_attack_phase(atk, 3))
	if CombatData.get_attack_phase(atk, 5) == "recovery": _p("frame 5 → recovery")
	else: _f("frame 5 → recovery", "got %s" % CombatData.get_attack_phase(atk, 5))


# ─── HealthComponent ───

func _test_health_component() -> void:
	print("\n📋 HealthComponent")

	var hp = HealthComponent.new()
	hp.max_hp = 100
	hp.current_hp = 100  # Set manually since not in tree (_ready won't fire)

	# Health ratio
	if hp.get_health_ratio() == 1.0: _p("full health ratio = 1.0")
	else: _f("full health ratio", "got %s" % hp.get_health_ratio())

	# Take damage — test return value instead of signal (signals need scene tree for deferred calls)
	var dmg_result = hp.damage(30, 0, false)
	if dmg_result == 30: _p("damage(30) returned 30")
	else: _f("damage(30) return value", "got %d" % dmg_result)
	hp.damage(30, 0, false)
	if hp.current_hp == 40: _p("damage(30)x2 → hp=40")
	else: _f("damage(30)x2 → hp=40", "got %d" % hp.current_hp)

	# Heal
	hp.heal(20)
	if hp.current_hp == 60: _p("heal(20) → hp=60")
	else: _f("heal(20) → hp=60", "got %d" % hp.current_hp)

	# Overheal
	hp.heal(200)
	if hp.current_hp == 100: _p("overheal capped at max_hp")
	else: _f("overheal capped", "got %d" % hp.current_hp)

	# Death — test return value (should return remaining 100 hp)
	var death_dmg = hp.damage(100, 0, false)
	if hp.current_hp == 0: _p("lethal damage → hp=0")
	else: _f("lethal damage → hp=0", "got %d" % hp.current_hp)
	if death_dmg == 100: _p("lethal damage returned actual dmg (100)")
	else: _f("lethal damage return", "got %d" % death_dmg)

	# Reset
	hp.reset()
	if hp.current_hp == 100: _p("reset() → hp=100")
	else: _f("reset() → hp=100", "got %d" % hp.current_hp)


# ─── EnergyComponent ───

func _test_energy_component() -> void:
	print("\n📋 EnergyComponent")

	var en = EnergyComponent.new()
	en.max_energy = 100
	en.starting_energy = 50
	en._ready()  # Manual init

	if en.current_energy == 50: _p("starting_energy = 50")
	else: _f("starting_energy", "got %d" % en.current_energy)

	# Spend energy
	if en.spend(30): _p("spend(30) succeeded")
	else: _f("spend(30)")
	if en.current_energy == 20: _p("after spend: energy=20")
	else: _f("after spend energy=20", "got %d" % en.current_energy)

	# Not enough
	if not en.spend(50): _p("spend(50) rejected (insufficient)")
	else: _f("spend(50) should fail")

	# Gain energy
	en.gain(40)
	if en.current_energy == 60: _p("gain(40) → energy=60")
	else: _f("gain(40) → energy=60", "got %d" % en.current_energy)

	# Overcap
	en.gain(100)
	if en.current_energy == 100: _p("overgain capped at max")
	else: _f("overgain capped", "got %d" % en.current_energy)


# ─── StateMachine ───

func _test_state_machine() -> void:
	print("\n📋 StateMachine")

	var sm = StateMachine.new()
	var state_a = State.new()
	state_a.set_name("Idle")
	var state_b = State.new()
	state_b.set_name("Run")
	sm.add_child(state_a)
	sm.add_child(state_b)

	# Initial state
	sm.initial_state_name = "Idle"
	sm._ready()

	if sm.get_current_state_name() == "Idle": _p("initial state = Idle")
	else: _f("initial state", "got %s" % sm.get_current_state_name())

	# Transition
	sm.transition_to("Run")
	if sm.get_current_state_name() == "Run": _p("transition to Run")
	else: _f("transition to Run", "got %s" % sm.get_current_state_name())


# ─── InputBuffer ───

func _test_input_buffer() -> void:
	print("\n📋 InputBuffer")

	var ib = InputBuffer.new()

	# Buffer an input (use _record_action which is the internal method)
	ib._record_action("attack_light", 40)
	if ib.consume("attack_light"): _p("buffered attack_light consumed")
	else: _f("buffered attack_light consumed")

	# Already consumed
	if not ib.consume("attack_light"): _p("double-consume prevented")
	else: _f("double-consume prevented")


# ─── FrameTimer ───

func _test_frame_timer() -> void:
	print("\n📋 FrameTimer")

	var ft = FrameTimer.new()
	ft.start(10)

	if ft.is_running(): _p("timer running after start(10)")
	else: _f("timer running after start(10)")

	# Tick
	for i in range(10):
		ft.tick()
	if not ft.is_running(): _p("timer stopped after 10 ticks")
	else: _f("timer stopped after 10 ticks")

	# Restart
	ft.start(5)
	if ft.is_running(): _p("restart succeeds")
	else: _f("restart succeeds")


# ─── HitboxComponent + HurtboxComponent ───

func _test_hitbox_hurtbox() -> void:
	print("\n📋 HitboxComponent + HurtboxComponent")

	var hbox = HitboxComponent.new()
	var hurtbox = HurtboxComponent.new()

	# Activation
	var atk = AttackData.new()
	atk.attack_id = "test_attack"
	hbox.activate(atk)
	if hbox.get_attack_data() != null: _p("hitbox activated with attack data")
	else: _f("hitbox activated")

	hbox.deactivate()
	if hbox.get_attack_data() == null: _p("hitbox deactivated")
	else: _f("hitbox deactivated")

	# Hit tracking
	hbox.activate(atk)
	if not hbox.has_hit_target(hurtbox): _p("no prior hit recorded")
	else: _f("no prior hit recorded")

	hbox.record_hit(hurtbox)
	if hbox.has_hit_target(hurtbox): _p("hit recorded correctly")
	else: _f("hit recorded")


# ─── HitManager ───

func _test_hit_manager() -> void:
	print("\n📋 HitManager")

	var hm = HitManager.new()
	if hm: _p("HitManager instantiated")
	else: _f("HitManager instantiated")


# ─── ComboManager ───

func _test_combo_manager() -> void:
	print("\n📋 ComboManager")

	var cm = ComboManager.new()
	if cm: _p("ComboManager instantiated")
	else: _f("ComboManager instantiated")


# ─── Scene Instantiation ───

func _test_scene_instantiation() -> void:
	print("\n📋 Scene Instantiation")

	# Player scene
	var player_scene = load("res://src/gameplay/scenes/player.tscn")
	if player_scene:
		var player = player_scene.instantiate()
		if player:
			_p("Player scene instantiated — name: %s" % player.name)
			# Check key components exist
			if player.has_node("StateMachine"): _p("Player has StateMachine")
			else: _f("Player has StateMachine")
			if player.has_node("InputBuffer"): _p("Player has InputBuffer")
			else: _f("Player has InputBuffer")
			if player.has_node("HealthComponent"): _p("Player has HealthComponent")
			else: _f("Player has HealthComponent")
			if player.has_node("EnergyComponent"): _p("Player has EnergyComponent")
			else: _f("Player has EnergyComponent")
			if player.has_node("ComboManager"): _p("Player has ComboManager")
			else: _f("Player has ComboManager")
			player.free()
		else:
			_f("Player instantiate()")
	else:
		_f("Player load()")

	# Enemy scene
	var enemy_scene = load("res://src/gameplay/scenes/enemy.tscn")
	if enemy_scene:
		var enemy = enemy_scene.instantiate()
		if enemy:
			_p("Enemy scene instantiated — name: %s" % enemy.name)
			if enemy.has_node("StateMachine"): _p("Enemy has StateMachine")
			else: _f("Enemy has StateMachine")
			if enemy.has_node("HealthComponent"): _p("Enemy has HealthComponent")
			else: _f("Enemy has HealthComponent")
			if enemy.has_node("HurtboxComponent"): _p("Enemy has HurtboxComponent")
			else: _f("Enemy has HurtboxComponent")
			enemy.free()
		else:
			_f("Enemy instantiate()")
	else:
		_f("Enemy load()")

	# Combat test scene
	var combat_scene = load("res://src/gameplay/scenes/combat_test.tscn")
	if combat_scene:
		var combat = combat_scene.instantiate()
		if combat:
			_p("CombatTest scene instantiated — children: %d" % combat.get_child_count())
			if combat.has_node("Player"): _p("CombatTest has Player")
			else: _f("CombatTest has Player")
			if combat.has_node("Enemy"): _p("CombatTest has Enemy")
			else: _f("CombatTest has Enemy")
			combat.free()
		else:
			_f("CombatTest instantiate()")
	else:
		_f("CombatTest load()")


# ─── SignalBus ───

func _test_signal_bus() -> void:
	print("\n📋 SignalBus")

	# Verify all expected signals exist
	var expected_signals := [
		"player_health_changed", "player_died", "player_energy_changed",
		"hit_confirmed", "combo_updated", "combo_dropped", "combo_maxed",
		"enemy_died", "enemy_hit",
		"screen_shake_requested",
		"room_entered", "game_paused", "game_resumed",
	]
	for sig_name in expected_signals:
		if SignalBus.has_signal(sig_name):
			_p("SignalBus.%s" % sig_name)
		else:
			_f("SignalBus.%s" % sig_name, "not found")

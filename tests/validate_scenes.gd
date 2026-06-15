extends Node2D

## Validate scene files by attempting to load and instantiate them.
## Run as the main scene: godot --headless res://tests/validate_scenes.tscn

func _ready() -> void:
	print("\n" + "=".repeat(60))
	print("格斗萌主 — Headless Scene Validation")
	print("=".repeat(60))

	var scenes := [
		"res://src/gameplay/scenes/player.tscn",
		"res://src/gameplay/scenes/enemy.tscn",
		"res://src/gameplay/scenes/combat_test.tscn",
	]

	for path in scenes:
		print("\n📋 Loading: " + path)
		var scene := load(path)
		if scene == null:
			print("  ❌ load() returned null")
			continue
		print("  ✅ load() succeeded — type: " + scene.get_class())
		if scene is PackedScene:
			var instance = scene.instantiate()
			if instance:
				print("  ✅ instantiate() succeeded — root: " + instance.name)
				var children = instance.get_children()
				print("  📦 Children: %d" % children.size())
				for child in children:
					print("    - %s (%s)" % [child.name, child.get_class()])
				instance.free()
			else:
				print("  ❌ instantiate() returned null")

	# Autoload singletons
	print("\n📋 Testing Autoload Singletons:")
	if CombatData:
		print("  ✅ CombatData: " + CombatData.get_class())
		if CombatData.player_config:
			print("    player_config: move_speed=%.1f" % CombatData.player_config.move_speed)
		if CombatData.get_enemy_data("goblin"):
			print("    goblin: max_hp=%d" % CombatData.get_enemy_data("goblin").max_hp)
	else:
		print("  ❌ CombatData: null")

	if SignalBus:
		print("  ✅ SignalBus: " + SignalBus.get_class())
	else:
		print("  ❌ SignalBus: null")

	if GameManager:
		print("  ✅ GameManager: " + GameManager.get_class())
	else:
		print("  ❌ GameManager: null")

	if AudioManager:
		print("  ✅ AudioManager: " + AudioManager.get_class())
	else:
		print("  ❌ AudioManager: null")

	# Core class tests
	print("\n📋 Testing Core Class Instantiation:")
	var tests := {
		"StateMachine": func(): return StateMachine.new(),
		"InputBuffer": func(): return InputBuffer.new(),
		"FrameTimer": func(): return FrameTimer.new(),
		"HealthComponent": func(): return HealthComponent.new(),
		"EnergyComponent": func(): return EnergyComponent.new(),
		"MovementComponent": func(): return MovementComponent.new(),
		"HitboxComponent": func(): return HitboxComponent.new(),
		"HurtboxComponent": func(): return HurtboxComponent.new(),
		"HitManager": func(): return HitManager.new(),
		"ComboManager": func(): return ComboManager.new(),
		"HitStopManager": func(): return HitStopManager.new(),
		"PlayerConfig": func(): return PlayerConfig.new(),
		"EnemyData": func(): return EnemyData.new(),
		"AttackData": func(): return AttackData.new(),
		"ComboData": func(): return ComboData.new(),
	}
	for name in tests:
		var obj = tests[name].call()
		if obj:
			print("  ✅ %s.new() succeeded" % name)
			if not obj is RefCounted:
				obj.free()
		else:
			print("  ❌ %s.new() failed" % name)

	print("\n" + "=".repeat(60))
	print("Validation complete. Exiting...")
	get_tree().quit()

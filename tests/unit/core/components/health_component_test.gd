extends "res://addons/gut/test.gd"

var health: HealthComponent

func before_each() -> void:
	health = HealthComponent.new()
	health.max_hp = 100
	add_child(health)  # triggers _ready

func after_each() -> void:
	health.queue_free()

func test_initial_hp() -> void:
	assert_eq(health.current_hp, 100, "HP should start at max_hp")

func test_damage_reduces_hp() -> void:
	health.damage(30)
	assert_eq(health.current_hp, 70, "HP should be reduced by damage amount")

func test_damage_returns_actual() -> void:
	var dealt: int = health.damage(30)
	assert_eq(dealt, 30, "damage() should return actual damage dealt")

func test_damage_capped_at_zero() -> void:
	health.max_hp = 10
	health.current_hp = 10
	health.damage(999)
	assert_eq(health.current_hp, 0, "HP should not go below 0")

func test_died_signal() -> void:
	var died_count: int = 0
	health.died.connect(func(): died_count += 1)
	health.damage(100)
	assert_eq(died_count, 1, "died signal should emit when HP reaches 0")

func test_damage_at_zero_returns_zero() -> void:
	health.damage(100)
	var dealt: int = health.damage(10)
	assert_eq(dealt, 0, "damage() at 0 HP should return 0")

func test_health_changed_signal() -> void:
	var signal_fired: bool = false
	var sig_current: int = 0
	var sig_max: int = 0
	health.health_changed.connect(func(c, m): signal_fired = true; sig_current = c; sig_max = m)
	health.damage(30)
	assert_true(signal_fired, "health_changed should fire on damage")
	assert_eq(sig_current, 70, "health_changed current should be 70")
	assert_eq(sig_max, 100, "health_changed maximum should be 100")

func test_damage_taken_signal() -> void:
	var sig_amount: int = 0
	var sig_type: int = -1
	var sig_crit: bool = true
	health.damage_taken.connect(func(a, t, c): sig_amount = a; sig_type = t; sig_crit = c)
	health.damage(25, 1, true)
	assert_eq(sig_amount, 25, "damage_taken amount should be 25")
	assert_eq(sig_type, 1, "damage_taken type should be 1")
	assert_true(sig_crit, "damage_taken is_crit should be true")

func test_invincible_blocks_damage() -> void:
	health.set_invincible(10)
	var dealt: int = health.damage(30)
	assert_eq(dealt, 0, "Damage should be blocked when invincible")
	assert_eq(health.current_hp, 100, "HP should not change when invincible")

func test_invincible_decrements() -> void:
	health.set_invincible(3)
	assert_true(health.is_invincible(), "Should be invincible initially")
	# Simulate 3 physics frames
	health._physics_process(1.0 / 60.0)
	health._physics_process(1.0 / 60.0)
	health._physics_process(1.0 / 60.0)
	assert_false(health.is_invincible(), "Should not be invincible after frames expire")

func test_invincible_max_frames() -> void:
	health.set_invincible(5)
	health.set_invincible(3)
	assert_true(health.is_invincible(), "Should still be invincible")
	# 3 frames should not expire the 5-frame invincibility
	health._physics_process(1.0 / 60.0)
	health._physics_process(1.0 / 60.0)
	health._physics_process(1.0 / 60.0)
	assert_true(health.is_invincible(), "set_invincible(3) should not shorten 5-frame invincibility")

func test_heal() -> void:
	health.damage(40)
	var healed: int = health.heal(20)
	assert_eq(healed, 20, "heal() should return actual amount healed")
	assert_eq(health.current_hp, 80, "HP should be restored by heal amount")

func test_heal_at_max() -> void:
	var healed: int = health.heal(20)
	assert_eq(healed, 0, "heal() at max HP should return 0")
	assert_eq(health.current_hp, 100, "HP should not exceed max_hp")

func test_reset() -> void:
	health.damage(50)
	health.reset()
	assert_eq(health.current_hp, 100, "reset() should restore HP to max_hp")

func test_get_health_ratio() -> void:
	health.damage(50)
	assert_eq(health.get_health_ratio(), 0.5, "health ratio should be 0.5 at half HP")

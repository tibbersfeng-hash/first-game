extends "res://addons/gut/test.gd"

var energy: EnergyComponent

func before_each() -> void:
	energy = EnergyComponent.new()
	energy.max_energy = 100
	energy.starting_energy = 0
	energy.special_threshold = 30
	energy.dodge_cost = 8
	energy.heavy_cost = 5
	add_child(energy)  # triggers _ready

func after_each() -> void:
	energy.queue_free()

func test_initial_energy() -> void:
	assert_eq(energy.current_energy, 0, "Energy should start at starting_energy")

func test_gain_increases() -> void:
	energy.gain(30)
	assert_eq(energy.current_energy, 30, "Energy should increase by gain amount")

func test_gain_clamps_to_max() -> void:
	energy.gain(200)
	assert_eq(energy.current_energy, 100, "Energy should not exceed max_energy")

func test_gain_zero_ignored() -> void:
	energy.gain(0)
	assert_eq(energy.current_energy, 0, "gain(0) should not change energy")

func test_gain_negative_ignored() -> void:
	energy.gain(-5)
	assert_eq(energy.current_energy, 0, "gain(-5) should not change energy")

func test_spend_success() -> void:
	energy.gain(50)
	var result: bool = energy.spend(30)
	assert_true(result, "spend() should succeed with sufficient energy")
	assert_eq(energy.current_energy, 20, "Energy should be reduced by spend amount")

func test_spend_failure_insufficient() -> void:
	energy.gain(10)
	var result: bool = energy.spend(30)
	assert_false(result, "spend() should fail with insufficient energy")
	assert_eq(energy.current_energy, 10, "Energy should not change on failed spend")

func test_can_special() -> void:
	energy.gain(35)
	assert_true(energy.can_special(), "Should be able to use special with 35 energy (threshold 30)")

func test_can_special_not_enough() -> void:
	energy.gain(25)
	assert_false(energy.can_special(), "Should not be able to use special with 25 energy (threshold 30)")

func test_can_dodge() -> void:
	energy.gain(10)
	assert_true(energy.can_dodge(), "Should be able to dodge with 10 energy (cost 8)")

func test_can_heavy() -> void:
	energy.gain(6)
	assert_true(energy.can_heavy(), "Should be able to use heavy with 6 energy (cost 5)")

func test_can_spend() -> void:
	energy.gain(20)
	assert_true(energy.can_spend(15), "Should be able to spend 15 with 20 energy")
	assert_false(energy.can_spend(25), "Should not be able to spend 25 with 20 energy")

func test_energy_changed_signal() -> void:
	var signal_count: int = 0
	energy.energy_changed.connect(func(_c, _m): signal_count += 1)
	energy.gain(30)
	assert_eq(signal_count, 1, "energy_changed should fire on gain")
	energy.spend(10)
	assert_eq(signal_count, 2, "energy_changed should fire on spend")

func test_reset() -> void:
	energy.gain(50)
	energy.reset()
	assert_eq(energy.current_energy, 0, "reset() should restore starting_energy")

func test_get_energy_ratio() -> void:
	energy.gain(50)
	assert_eq(energy.get_energy_ratio(), 0.5, "Energy ratio should be 0.5 at half energy")

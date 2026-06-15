extends "res://addons/gut/test.gd"

## ComboManager tests use manually created ComboData + AttackData
## to avoid dependency on CombatData autoload loading .tres files.

var cm: ComboManager
var test_chain: ComboData

func before_each() -> void:
	cm = ComboManager.new()

	# Create a test combo chain: LIGHT → LIGHT → HEAVY
	test_chain = ComboData.new()
	test_chain.chain_name = "test_basic"
	test_chain.timeout_frames = 48

	var step1: AttackData = AttackData.new()
	step1.type = 0  # LIGHT
	step1.damage = 10
	step1.window_frames = 18

	var step2: AttackData = AttackData.new()
	step2.type = 0  # LIGHT
	step2.damage = 12
	step2.window_frames = 15

	var step3: AttackData = AttackData.new()
	step3.type = 1  # HEAVY
	step3.damage = 25
	step3.window_frames = 0  # use chain timeout

	test_chain.steps = [step1, step2, step3]

	# Inject test chain into CombatData if autoload is available
	# For unit testing, we directly set the combo_chains
	if CombatData:
		CombatData.combo_chains["test_basic"] = test_chain

	add_child(cm)  # triggers _ready

func after_each() -> void:
	cm.queue_free()
	if CombatData and CombatData.combo_chains.has("test_basic"):
		CombatData.combo_chains.erase("test_basic")

func test_try_attack_starts_combo() -> void:
	var result: bool = cm.try_attack(0)  # LIGHT
	assert_true(result, "try_attack(LIGHT) should start the test_basic combo")

func test_try_attack_wrong_type() -> void:
	# Remove all chains temporarily
	var saved_chains: Dictionary = CombatData.combo_chains.duplicate() if CombatData else {}
	if CombatData:
		CombatData.combo_chains.clear()
	var result: bool = cm.try_attack(0)
	assert_false(result, "try_attack should fail with no matching chain")
	# Restore
	if CombatData:
		CombatData.combo_chains = saved_chains

func test_combo_step_signal() -> void:
	var step_index: int = -1
	var step_data: AttackData = null
	cm.combo_step.connect(func(i, d): step_index = i; step_data = d)
	cm.try_attack(0)  # LIGHT
	assert_eq(step_index, 0, "First step index should be 0")
	assert_not_null(step_data, "step_data should not be null")
	if step_data:
		assert_eq(step_data.damage, 10, "First step damage should be 10")

func test_combo_finished_signal() -> void:
	var finished_chain: String = ""
	cm.combo_finished.connect(func(n): finished_chain = n)
	# Advance through all 3 steps
	cm.try_attack(0)   # Step 1: LIGHT
	cm.try_attack(0)   # Step 2: LIGHT
	cm.try_attack(1)   # Step 3: HEAVY → chain complete
	assert_eq(finished_chain, "test_basic", "combo_finished should fire with chain name")

func test_advance_combo() -> void:
	cm.try_attack(0)   # Step 1
	assert_eq(cm.get_combo_count(), 1, "Combo count should be 1 after first step")
	cm.try_attack(0)   # Step 2
	assert_eq(cm.get_combo_count(), 2, "Combo count should be 2 after second step")

func test_wrong_input_drops_combo() -> void:
	var dropped: bool = false
	cm.combo_dropped.connect(func(): dropped = true)
	cm.try_attack(0)   # Step 1: LIGHT — starts test_basic
	# Step 2 expects LIGHT, but we send HEAVY which doesn't start a new chain
	# First remove any HEAVY-starting chains
	if CombatData:
		var to_remove: Array[String] = []
		for key: String in CombatData.combo_chains:
			if key != "test_basic":
				to_remove.append(key)
		for key: String in to_remove:
			CombatData.combo_chains.erase(key)
	cm.try_attack(1)   # HEAVY — wrong input for test_basic step 2
	assert_true(dropped, "Wrong input should drop the combo")

func test_wrong_input_starts_new_chain() -> void:
	# Create a HEAVY-starting chain
	var heavy_chain: ComboData = ComboData.new()
	heavy_chain.chain_name = "test_power"
	heavy_chain.timeout_frames = 48
	var heavy_step: AttackData = AttackData.new()
	heavy_step.type = 1  # HEAVY
	heavy_step.damage = 30
	heavy_step.window_frames = 20
	heavy_chain.steps = [heavy_step]
	if CombatData:
		CombatData.combo_chains["test_power"] = heavy_chain

	cm.try_attack(0)   # Start test_basic with LIGHT
	cm.try_attack(1)   # HEAVY — wrong for test_basic step 2, but starts test_power
	assert_eq(cm.get_current_chain_name(), "test_power", "Should start new chain on wrong input")

	if CombatData:
		CombatData.combo_chains.erase("test_power")

func test_combo_timeout_drops() -> void:
	var dropped: bool = false
	cm.combo_dropped.connect(func(): dropped = true)
	cm.try_attack(0)  # Start combo
	# Simulate timeout by ticking the internal timer
	for _i in range(50):
		cm._combo_timer.tick()
	assert_true(dropped, "Combo should be dropped after timeout expires")

func test_get_current_step_data() -> void:
	cm.try_attack(0)
	var data: AttackData = cm.get_current_step_data()
	assert_not_null(data, "Should return step data during active combo")
	if data:
		assert_eq(data.damage, 10, "First step damage should be 10")

func test_get_current_step_data_no_combo() -> void:
	var data: AttackData = cm.get_current_step_data()
	assert_null(data, "Should return null when no combo is active")

func test_get_combo_count() -> void:
	assert_eq(cm.get_combo_count(), 0, "Combo count should be 0 when inactive")
	cm.try_attack(0)
	assert_eq(cm.get_combo_count(), 1, "Combo count should be 1 after first step")

# Combo manager — tracks combo chain state and timing
# ADR-006: All timing in integer frame counts, runs in _physics_process

extends Node

signal combo_step(step_index: int, step_data: AttackStepData)
signal combo_finished(chain_name: String)
signal combo_dropped

var _current_chain: ComboChainData = null
var _current_step: int = 0
var _combo_timer_frames: int = 0
var _is_active: bool = false

func _physics_process(_delta: float) -> void:
	if not _is_active:
		return

	_combo_timer_frames -= 1
	if _combo_timer_frames <= 0:
		_drop_combo()

func try_attack(attack_type: int) -> bool:
	# No active combo — start one based on first input
	if not _is_active:
		var chain: ComboChainData = _find_chain_for_input(attack_type)
		if chain == null:
			return false
		_start_combo(chain)
		return true

	# Active combo — try to advance
	var steps: Array[AttackStepData] = _current_chain.steps
	if _current_step >= steps.size():
		# Chain complete, start fresh
		_reset()
		return try_attack(attack_type)

	var expected: AttackStepData = steps[_current_step]
	if attack_type == expected.type:
		_advance_combo()
		return true
	else:
		# Wrong input for this chain — drop and try new chain
		var new_chain: ComboChainData = _find_chain_for_input(attack_type)
		if new_chain != null:
			_reset()
			_start_combo(new_chain)
			return true
		_drop_combo()
		return false

func get_current_step_data() -> AttackStepData:
	if not _is_active or _current_chain == null:
		return null
	var steps: Array[AttackStepData] = _current_chain.steps
	var idx: int = _current_step - 1
	if idx < 0 or idx >= steps.size():
		return null
	return steps[idx]

func get_current_chain_name() -> String:
	if _current_chain:
		return _current_chain.chain_name
	return ""

func _start_combo(chain: ComboChainData) -> void:
	_current_chain = chain
	_current_step = 0
	_is_active = true
	_advance_combo()

func _advance_combo() -> void:
	var steps: Array[AttackStepData] = _current_chain.steps
	var step_data: AttackStepData = steps[_current_step]
	_current_step += 1
	_combo_timer_frames = step_data.window_frames if step_data.window_frames > 0 else _current_chain.timeout_frames
	combo_step.emit(_current_step - 1, step_data)

	# Check if chain complete
	if _current_step >= steps.size():
		combo_finished.emit(_current_chain.chain_name)
		# Don't reset immediately — let combo timeout handle it
		_combo_timer_frames = _current_chain.timeout_frames

func _drop_combo() -> void:
	combo_dropped.emit()
	_reset()

func _reset() -> void:
	_current_chain = null
	_current_step = 0
	_combo_timer_frames = 0
	_is_active = false

func _find_chain_for_input(attack_type: int) -> ComboChainData:
	for chain_name: String in CombatData.combo_chains:
		var chain: ComboChainData = CombatData.combo_chains[chain_name]
		if chain.steps.size() > 0 and chain.steps[0].type == attack_type:
			return chain
	return null

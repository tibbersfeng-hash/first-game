# PROTOTYPE - NOT FOR PRODUCTION
# Combo manager — tracks combo chain state and timing
# Question: Can simplified 3-4 key combos feel satisfying?

extends Node

signal combo_step(step_index: int, step_data: Dictionary)
signal combo_finished(chain_name: String)
signal combo_dropped

var _current_chain: String = ""
var _current_step: int = 0
var _combo_timer: float = 0.0
var _is_active: bool = false

func _process(delta: float) -> void:
	if not _is_active:
		return

	_combo_timer -= delta
	if _combo_timer <= 0:
		_drop_combo()

func try_attack(attack_type: int) -> bool:
	# No active combo — start one based on first input
	if not _is_active:
		var chain: String = _find_chain_for_input(attack_type)
		if chain == "":
			return false
		_start_combo(chain)
		return true

	# Active combo — try to advance
	var chain_data: Array = CombatData.COMBO_CHAINS[_current_chain]
	if _current_step >= chain_data.size():
		# Chain complete, start fresh
		_reset()
		return try_attack(attack_type)

	var expected: Dictionary = chain_data[_current_step]
	if attack_type == expected["type"]:
		_advance_combo()
		return true
	else:
		# Wrong input for this chain — drop and try new chain
		var new_chain: String = _find_chain_for_input(attack_type)
		if new_chain != "":
			_reset()
			_start_combo(new_chain)
			return true
		_drop_combo()
		return false

func get_current_step_data() -> Dictionary:
	if not _is_active or _current_chain == "":
		return {}
	var chain_data: Array = CombatData.COMBO_CHAINS[_current_chain]
	if _current_step - 1 < 0 or _current_step - 1 >= chain_data.size():
		return {}
	return chain_data[_current_step - 1]

func _start_combo(chain: String) -> void:
	_current_chain = chain
	_current_step = 0
	_is_active = true
	_advance_combo()

func _advance_combo() -> void:
	var chain_data: Array = CombatData.COMBO_CHAINS[_current_chain]
	var step_data: Dictionary = chain_data[_current_step]
	_current_step += 1
	_combo_timer = step_data["window"] if step_data["window"] > 0 else CombatData.COMBO_TIMEOUT
	combo_step.emit(_current_step - 1, step_data)

	# Check if chain complete
	if _current_step >= chain_data.size():
		combo_finished.emit(_current_chain)
		# Don't reset immediately — let combo_timeout handle it
		_combo_timer = CombatData.COMBO_TIMEOUT

func _drop_combo() -> void:
	combo_dropped.emit()
	_reset()

func _reset() -> void:
	_current_chain = ""
	_current_step = 0
	_combo_timer = 0.0
	_is_active = false

func _find_chain_for_input(attack_type: int) -> String:
	for chain_name: String in CombatData.COMBO_CHAINS:
		var first_step: Dictionary = CombatData.COMBO_CHAINS[chain_name][0]
		if first_step["type"] == attack_type:
			return chain_name
	return ""

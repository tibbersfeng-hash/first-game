## Combo manager — tracks combo chain state and timing.
## ADR-006: All timing in integer frame counts, runs in _physics_process.
## Uses ComboData + AttackData from Resource configuration.
## Same-layer component: Player listens to signals and forwards to SignalBus.
class_name ComboManager
extends Node

signal combo_step(step_index: int, step_data: AttackData)
signal combo_finished(chain_name: String)
signal combo_dropped

var _current_chain: ComboData = null
var _current_step: int = 0
var _combo_timer: FrameTimer = null
var _is_active: bool = false

func _ready() -> void:
	_combo_timer = FrameTimer.new()
	_combo_timer.expired.connect(_on_combo_timer_expired)

func _physics_process(_delta: float) -> void:
	if not _is_active:
		return
	_combo_timer.tick()

## Try to process an attack input. Returns true if the attack was accepted.
func try_attack(attack_type: int) -> bool:
	if not _is_active:
		var chain: ComboData = _find_chain_for_input(attack_type)
		if chain == null:
			return false
		_start_combo(chain)
		return true

	# Active combo — try to advance
	var steps: Array[AttackData] = _current_chain.steps
	if _current_step >= steps.size():
		# Chain complete, start fresh
		_reset()
		return try_attack(attack_type)

	var expected: AttackData = steps[_current_step]
	if attack_type == expected.type:
		_advance_combo()
		return true
	else:
		# Wrong input for this chain — try new chain
		var new_chain: ComboData = _find_chain_for_input(attack_type)
		if new_chain != null:
			_reset()
			_start_combo(new_chain)
			return true
		_drop_combo()
		return false

## Get the current step's attack data, or null if no active combo.
func get_current_step_data() -> AttackData:
	if not _is_active or _current_chain == null:
		return null
	var steps: Array[AttackData] = _current_chain.steps
	var idx: int = _current_step - 1
	if idx < 0 or idx >= steps.size():
		return null
	return steps[idx]

## Get the current chain name, or empty string.
func get_current_chain_name() -> String:
	if _current_chain:
		return _current_chain.chain_name
	return ""

## Get the current combo count (number of successful steps).
func get_combo_count() -> int:
	if not _is_active:
		return 0
	return _current_step

func _start_combo(chain: ComboData) -> void:
	_current_chain = chain
	_current_step = 0
	_is_active = true
	_advance_combo()

func _advance_combo() -> void:
	var steps: Array[AttackData] = _current_chain.steps
	var step_data: AttackData = steps[_current_step]
	_current_step += 1
	# Start timer for this step's input window
	var window: int = step_data.window_frames if step_data.window_frames > 0 else _current_chain.timeout_frames
	_combo_timer.start(window)
	combo_step.emit(_current_step - 1, step_data)

	# Check if chain complete
	if _current_step >= steps.size():
		combo_finished.emit(_current_chain.chain_name)
		# Don't reset immediately — let combo timeout handle it
		_combo_timer.start(_current_chain.timeout_frames)

func _drop_combo() -> void:
	combo_dropped.emit()
	_reset()

func _reset() -> void:
	_current_chain = null
	_current_step = 0
	_combo_timer.stop()
	_is_active = false

func _on_combo_timer_expired() -> void:
	if _is_active:
		_drop_combo()

func _find_chain_for_input(attack_type: int) -> ComboData:
	for chain_name: String in CombatData.combo_chains:
		var chain: ComboData = CombatData.combo_chains[chain_name]
		if chain.steps.size() > 0 and chain.steps[0].type == attack_type:
			return chain
	return null

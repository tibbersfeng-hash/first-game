## Frame-precision input buffer for player inputs.
## ADR-006: all timing in integer frame counts.
## Input System GDD: max 2 buffered actions, same-type inputs overwrite older ones.
## Priority resolution: dodge > special > heavy > light > movement.
class_name InputBuffer
extends Node

const MAX_BUFFER_SIZE: int = 8
const DEFAULT_WINDOW_FRAMES: int = 8  # ~133ms at 60fps

var _buffer: Array[BufferedInput] = []
var _current_frame: int = 0

## Action categories for priority resolution and same-type overwrite logic.
## Per input-system GDD: dodge > special > heavy > light > movement
const PRIORITY_MAP: Dictionary = {
	"dodge": 80,
	"attack_special": 60,
	"attack_heavy": 50,
	"attack_light": 40,
	"jump": 30,
}

signal action_buffered(action_name: String, priority: int)

func _physics_process(_delta: float) -> void:
	_current_frame += 1
	_prune_expired()
	_poll_new_inputs()

## Check if a specific action was buffered within the given window.
## If found, marks it as consumed and returns true.
func consume(action_name: String, window_frames: int = DEFAULT_WINDOW_FRAMES) -> bool:
	for input_event: BufferedInput in _buffer:
		if input_event.action_name == action_name and input_event.is_within_window(_current_frame, window_frames):
			input_event.consumed = true
			return true
	return false

## Check if a specific action is currently held (for directional input).
func is_action_pressed(action_name: String) -> bool:
	return Input.is_action_pressed(action_name)

## Get the axis value for movement (for directional input).
func get_vector(negative_action: String, positive_action: String) -> float:
	return Input.get_axis(negative_action, positive_action)

## Get the highest-priority buffered action within the window.
## Returns the action name, or empty string if none available.
func consume_highest_priority(window_frames: int = DEFAULT_WINDOW_FRAMES) -> String:
	var best: BufferedInput = null
	for input_event: BufferedInput in _buffer:
		if input_event.consumed or not input_event.is_within_window(_current_frame, window_frames):
			continue
		if best == null or input_event.priority > best.priority:
			best = input_event
	if best:
		best.consumed = true
		return best.action_name
	return ""

## Clear all buffered inputs.
func clear() -> void:
	_buffer.clear()

## Get the number of active (unconsumed, unexpired) inputs.
func get_active_count() -> int:
	var count: int = 0
	for input_event: BufferedInput in _buffer:
		if not input_event.consumed and input_event.is_within_window(_current_frame, DEFAULT_WINDOW_FRAMES):
			count += 1
	return count

## Poll raw input for buffered actions this frame.
func _poll_new_inputs() -> void:
	for action_name: String in PRIORITY_MAP:
		if Input.is_action_just_pressed(action_name):
			_record_action(action_name, PRIORITY_MAP[action_name])

	if Input.is_action_just_pressed("jump"):
		_record_action("jump", PRIORITY_MAP.get("jump", 30))

## Record a new buffered input, applying same-type overwrite.
func _record_action(action_name: String, priority: int) -> void:
	# Same-type overwrite: remove older unconsumed input of the same action
	for i: int in range(_buffer.size() - 1, -1, -1):
		if _buffer[i].action_name == action_name and not _buffer[i].consumed:
			_buffer.remove_at(i)
			break

	var new_input: BufferedInput = BufferedInput.new(action_name, _current_frame, priority)
	_buffer.append(new_input)
	action_buffered.emit(action_name, priority)

	# Enforce max buffer size (drop oldest)
	while _buffer.size() > MAX_BUFFER_SIZE:
		_buffer.pop_front()

## Remove consumed and expired inputs from the buffer.
func _prune_expired() -> void:
	var max_window: int = DEFAULT_WINDOW_FRAMES * 2  # safety margin
	for i: int in range(_buffer.size() - 1, -1, -1):
		if _buffer[i].consumed or _buffer[i].age(_current_frame) > max_window:
			_buffer.remove_at(i)

## Generic state machine for all entities.
## Manages current state, transitions, and frame_in_state counter.
## ADR-006: physics_update() is called from _physics_process.
## States are child nodes of type State, indexed by name.
class_name StateMachine
extends Node

## The name of the state to enter on _ready. Case-insensitive lookup.
@export var initial_state_name: String = ""

var current_state: State = null
var states: Dictionary = {}  # String -> State
var frame_in_state: int = 0

signal state_entered(state_name: String)
signal state_exited(state_name: String)

func _ready() -> void:
	_collect_states()
	if initial_state_name != "" and states.has(initial_state_name.to_lower()):
		transition_to(initial_state_name)

func _physics_process(delta: float) -> void:
	if current_state == null:
		return
	frame_in_state += 1
	current_state.frame_in_state = frame_in_state
	current_state.physics_update(delta)

func _unhandled_input(event: InputEvent) -> void:
	if current_state:
		current_state.handle_input(event)

## Transition to the named state. No-op if state doesn't exist or is already current.
func transition_to(state_name: String) -> void:
	var key: String = state_name.to_lower()
	if not states.has(key):
		push_warning("StateMachine: unknown state '%s'" % state_name)
		return
	var new_state: State = states[key]
	if new_state == current_state:
		return
	if current_state:
		state_exited.emit(current_state.name)
		current_state.exit()
	current_state = new_state
	frame_in_state = 0
	current_state.frame_in_state = 0
	current_state.enter()
	state_entered.emit(current_state.name)

## Get a state by name. Returns null if not found.
func get_state(state_name: String) -> State:
	return states.get(state_name.to_lower(), null)

## Get the current state name, or empty string if no state.
func get_current_state_name() -> String:
	if current_state:
		return current_state.name
	return ""

## Collect all State child nodes into the states dictionary.
func _collect_states() -> void:
	for child in get_children():
		if child is State:
			var key: String = child.name.to_lower()
			states[key] = child
			child.state_machine = self

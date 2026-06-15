## Base class for all entity states.
## Subclasses override enter(), exit(), physics_update(), and handle_input().
## ADR-006: frame_in_state is an integer counter, incremented each physics frame.
## States are child nodes of StateMachine, discovered via get_children() in _ready().
class_name State
extends Node

## Reference to the owning StateMachine (set during _collect_states).
var state_machine: StateMachine = null

## Number of physics frames since this state was entered.
## Managed by StateMachine — reset on transition, incremented each tick.
var frame_in_state: int = 0

## Called once when this state becomes the active state.
## Override in subclasses. Reset per-state variables here.
func enter() -> void:
	pass

## Called once when this state is exited.
## Override in subclasses. Clean up per-state resources here.
func exit() -> void:
	pass

## Called every _physics_process tick while this state is active.
## ADR-006: delta is provided but frame_in_state should be preferred for timing.
func physics_update(_delta: float) -> void:
	pass

## Called to handle input events. Return true if the input was consumed.
func handle_input(_event: InputEvent) -> bool:
	return false

## Transition to another state by name.
## Convenience method that delegates to StateMachine.transition_to().
func transition_to(state_name: String) -> void:
	if state_machine:
		state_machine.transition_to(state_name)

## Get the owner entity (the CharacterBody2D or Node2D that owns the StateMachine).
func get_owner_entity() -> Node:
	return state_machine.owner if state_machine else null

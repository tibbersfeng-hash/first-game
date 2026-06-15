## Reusable health management component.
## ADR-004: max_hp loaded from Resource configuration.
## Attaches as a child node to any entity that needs HP tracking.
## ADR-006: invincibility frames are frame-based, decrementing in _physics_process.
class_name HealthComponent
extends Node

@export var max_hp: int = 100

var current_hp: int = 0
var _invincible_frames_remaining: int = 0

signal health_changed(current: int, maximum: int)
signal died
signal damage_taken(amount: int, damage_type: int, is_crit: bool)

func _ready() -> void:
	current_hp = max_hp

func _physics_process(_delta: float) -> void:
	if _invincible_frames_remaining > 0:
		_invincible_frames_remaining -= 1

## Apply damage to this entity. Returns actual damage dealt.
## If invincible, returns 0. Emits signals for cross-layer consumers.
func damage(amount: int, damage_type: int = 0, is_crit: bool = false) -> int:
	if _invincible_frames_remaining > 0:
		return 0
	if current_hp <= 0:
		return 0
	var actual: int = mini(amount, current_hp)
	current_hp -= actual
	damage_taken.emit(actual, damage_type, is_crit)
	health_changed.emit(current_hp, max_hp)
	if current_hp <= 0:
		died.emit()
	return actual

## Heal this entity. Returns actual amount healed.
func heal(amount: int) -> int:
	var actual: int = mini(amount, max_hp - current_hp)
	current_hp += actual
	if actual > 0:
		health_changed.emit(current_hp, max_hp)
	return actual

## Set invincibility for N frames (ADR-006: frame-based i-frames).
## Takes the max of current and new frames to prevent shortening existing i-frames.
func set_invincible(frames: int) -> void:
	_invincible_frames_remaining = maxi(_invincible_frames_remaining, frames)

## Check if currently in invincibility frames.
func is_invincible() -> bool:
	return _invincible_frames_remaining > 0

## Reset health to max (for respawn/restart).
func reset() -> void:
	current_hp = max_hp
	_invincible_frames_remaining = 0
	health_changed.emit(current_hp, max_hp)

## Get health as a 0.0-1.0 ratio.
func get_health_ratio() -> float:
	if max_hp == 0:
		return 0.0
	return float(current_hp) / float(max_hp)

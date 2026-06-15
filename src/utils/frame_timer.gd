## Frame-based countdown timer per ADR-006.
## Increments on explicit tick() calls; does NOT use _process or _physics_process.
## Extends RefCounted (not Node) for zero scene tree overhead.
## Callers call tick() from their own _physics_process.
class_name FrameTimer
extends RefCounted

var _remaining: int = 0
var _duration: int = 0
var _is_running: bool = false

signal expired

## Start the timer for the given number of frames.
## Idempotent: calling again resets the countdown.
func start(frame_count: int) -> void:
	_duration = maxi(frame_count, 0)
	_remaining = _duration
	_is_running = _remaining > 0

## Advance the timer by one frame. Call from _physics_process.
## Emits expired when the countdown reaches zero.
func tick() -> void:
	if not _is_running:
		return
	_remaining -= 1
	if _remaining <= 0:
		_remaining = 0
		_is_running = false
		expired.emit()

## Returns true if the timer is actively counting down.
func is_running() -> bool:
	return _is_running

## Returns the number of frames remaining.
func get_remaining() -> int:
	return _remaining

## Returns the number of frames elapsed since start.
func get_elapsed() -> int:
	return _duration - _remaining

## Returns progress as a 0.0-1.0 ratio (1.0 = complete).
func get_progress() -> float:
	if _duration == 0:
		return 1.0
	return 1.0 - (float(_remaining) / float(_duration))

## Stop the timer immediately.
func stop() -> void:
	_remaining = 0
	_is_running = false

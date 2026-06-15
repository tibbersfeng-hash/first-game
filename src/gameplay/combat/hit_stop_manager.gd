## Hit-stop manager — freezes game time on impact for juice.
## Uses _process (NOT _physics_process) intentionally:
## When Engine.time_scale = 0, the physics server accumulates no time,
## so _physics_process does not fire. This manager must count down during
## the frozen state, therefore it uses _process with PROCESS_MODE_ALWAYS.
class_name HitStopManager
extends Node

var _hit_stop_frames: int = 0

func _ready() -> void:
	process_mode = Node.PROCESS_MODE_ALWAYS  # runs even when paused

## Apply hit-stop for the given number of frames.
func apply_hit_stop(frames: int) -> void:
	if frames <= 0:
		return
	_hit_stop_frames = frames
	Engine.time_scale = CombatData.player_config.hit_stop_time_scale if CombatData.player_config else 0.0

func _process(_delta: float) -> void:
	if _hit_stop_frames > 0:
		_hit_stop_frames -= 1
		if _hit_stop_frames <= 0:
			Engine.time_scale = 1.0

## Check if currently in hit-stop.
func is_in_hit_stop() -> bool:
	return _hit_stop_frames > 0

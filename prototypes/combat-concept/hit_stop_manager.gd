# PROTOTYPE - NOT FOR PRODUCTION
# Hit-stop manager — freezes game time on impact for juice
# Question: Can hit-stop + shake create satisfying Q-version combat feel?

extends Node

var _hit_stop_frames: int = 0
var _original_time_scale: float = 1.0

func _ready() -> void:
	process_mode = Node.PROCESS_MODE_ALWAYS  # runs even when paused

func apply_hit_stop(frames: int) -> void:
	if frames <= 0:
		return
	_hit_stop_frames = frames
	Engine.time_scale = CombatData.HIT_STOP_SCALE

func _process(_delta: float) -> void:
	if _hit_stop_frames > 0:
		_hit_stop_frames -= 1
		if _hit_stop_frames <= 0:
			Engine.time_scale = 1.0

func is_in_hit_stop() -> bool:
	return _hit_stop_frames > 0

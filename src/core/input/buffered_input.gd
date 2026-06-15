## A single buffered input with frame-based age tracking.
## ADR-006: age is tracked in integer frames, not seconds.
## RefCounted data class — no scene tree overhead.
class_name BufferedInput
extends RefCounted

var action_name: String = ""
var frame_recorded: int = 0
var priority: int = 0
var consumed: bool = false

func _init(p_action: String = "", p_frame: int = 0, p_priority: int = 0) -> void:
	action_name = p_action
	frame_recorded = p_frame
	priority = p_priority

## Returns the number of frames since this input was recorded.
func age(current_frame: int) -> int:
	return current_frame - frame_recorded

## Returns true if this input is within the given window and not yet consumed.
func is_within_window(current_frame: int, window_frames: int) -> bool:
	return age(current_frame) <= window_frames and not consumed

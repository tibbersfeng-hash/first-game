# PROTOTYPE - NOT FOR PRODUCTION
# Screen shake — camera juice on hit
# Question: Can screen shake amplify Q-version combat satisfaction?

extends Camera2D

var _shake_intensity: float = 0.0
var _shake_decay: float = 15.0
var _shake_offset: Vector2 = Vector2.ZERO

@onready var _original_offset: Vector2 = offset

func shake(intensity: float) -> void:
	_shake_intensity = maxf(_shake_intensity, intensity)

func _process(delta: float) -> void:
	if _shake_intensity > 0.1:
		_shake_offset = Vector2(
			randf_range(-_shake_intensity, _shake_intensity),
			randf_range(-_shake_intensity, _shake_intensity)
		)
		_shake_intensity = lerpf(_shake_intensity, 0.0, _shake_decay * delta)
		offset = _original_offset + _shake_offset
	else:
		_shake_intensity = 0.0
		offset = _original_offset

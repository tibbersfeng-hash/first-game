# Screen shake — camera juice on hit
# ADR-003: Listens to SignalBus.screen_shake_requested
# ADR-006: Runs in _physics_process for frame-consistent behavior

extends Camera2D

var _shake_intensity: float = 0.0
var _shake_decay_per_tick: float = 0.25  # 15.0 / 60.0
var _shake_offset: Vector2 = Vector2.ZERO

@onready var _original_offset: Vector2 = offset

func _ready() -> void:
	SignalBus.screen_shake_requested.connect(_on_screen_shake_requested)

func _on_screen_shake_requested(intensity: float) -> void:
	shake(intensity)

func shake(intensity: float) -> void:
	_shake_intensity = maxf(_shake_intensity, intensity)

func _physics_process(_delta: float) -> void:
	if _shake_intensity > 0.1:
		_shake_offset = Vector2(
			randf_range(-_shake_intensity, _shake_intensity),
			randf_range(-_shake_intensity, _shake_intensity)
		)
		_shake_intensity = lerpf(_shake_intensity, 0.0, _shake_decay_per_tick)
		offset = _original_offset + _shake_offset
	else:
		_shake_intensity = 0.0
		offset = _original_offset

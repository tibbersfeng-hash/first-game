# Damage number visual configuration
class_name DamageNumberConfig
extends Resource

@export var font_size: int = 32
@export var crit_font_size: int = 48
@export var lifetime_frames: int = 48         # 0.8s at 60fps
@export var rise_speed: float = -80.0
@export var spread: float = 30.0
@export var color_light: Color = Color.WHITE
@export var color_heavy: Color = Color.YELLOW
@export var color_special: Color = Color(1.0, 0.3, 0.3)
@export var color_crit: Color = Color(1.0, 0.84, 0.0)
@export var outline_size: int = 3

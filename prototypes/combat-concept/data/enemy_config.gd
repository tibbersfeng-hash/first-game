# Enemy configuration data
class_name EnemyConfig
extends Resource

@export var max_health: int = 500
@export var gravity: float = 980.0
@export var light_knockback: float = 50.0
@export var heavy_knockback: float = 120.0
@export var special_knockback: float = 200.0
@export var friction: float = 300.0
@export var respawn_frames: int = 18          # 0.3s at 60fps

## Enemy definition data — attributes, behavior, and combat parameters.
## Referenced in ADR-004. Each enemy type has its own .tres instance.
class_name EnemyData
extends Resource

@export var enemy_id: String = ""
@export var display_name: String = ""
@export var max_hp: int = 100
@export var base_damage: int = 10
@export var move_speed: float = 100.0
@export var attack_range: float = 50.0
@export var chase_range: float = 200.0
@export var decision_interval_frames: int = 60
@export var attack_data: Array[AttackData] = []
@export var category: String = "minion"  # minion | elite | boss
@export var gravity: float = 980.0
@export var light_knockback: float = 50.0
@export var heavy_knockback: float = 120.0
@export var special_knockback: float = 200.0
@export var friction: float = 300.0
@export var respawn_frames: int = 18  # 0.3s at 60fps

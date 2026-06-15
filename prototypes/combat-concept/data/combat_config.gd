# Top-level combat configuration — references other configs
class_name CombatConfig
extends Resource

@export var hit_stop_time_scale: float = 0.0
@export var player_config: PlayerConfig = null
@export var enemy_config: EnemyConfig = null
@export var damage_number_config: DamageNumberConfig = null

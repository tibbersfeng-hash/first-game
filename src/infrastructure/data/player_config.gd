## Player configuration data — movement, combat, and energy parameters.
class_name PlayerConfig
extends Resource

@export var move_speed: float = 300.0
@export var jump_velocity: float = -500.0
@export var gravity: float = 980.0
@export var attack_push_speed: float = 50.0
@export var combo_damage_bonus_pct: float = 0.1
@export var crit_chance: float = 0.15
@export var crit_multiplier: float = 1.5
@export var air_control_ratio: float = 0.7
@export var coyote_time_frames: int = 5
@export var jump_buffer_frames: int = 9

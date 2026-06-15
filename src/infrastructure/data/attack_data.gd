## Single attack step definition with frame data.
## ADR-004: all values have type annotations and default values.
## ADR-006: all timing in integer frame counts.
class_name AttackData
extends Resource

@export var attack_id: String = ""
@export var display_name: String = ""
@export var type: int = 0  # CombatData.AttackType enum value
@export var damage: int = 10
@export var startup_frames: int = 2
@export var active_frames: int = 2
@export var recovery_frames: int = 3
@export var energy_cost: int = 0
@export var energy_gain_on_hit: int = 5
@export var cancel_priority: int = 0
@export var hit_stop_frames: int = 3
@export var shake_intensity: float = 0.0
@export var hitbox_offset: Vector2 = Vector2(30, -20)
@export var hitbox_size: Vector2 = Vector2(50, 40)
@export var window_frames: int = 18  # Combo input window in frames (0 = use chain timeout)
@export var anim_name: String = ""

# Attack step data — single attack within a combo chain
# All timing values are in integer frames (60fps)
class_name AttackStepData
extends Resource

@export var type: int = 0                    # CombatData.AttackType enum value
@export var damage: int = 10
@export var hit_stop_frames: int = 3
@export var shake_intensity: float = 0.0
@export var anim_name: String = ""
@export var window_frames: int = 18          # Combo input window in frames (0 = use chain timeout)
@export var startup_frames: int = 2          # Frames before hitbox activates
@export var active_frames: int = 2           # Frames hitbox is enabled
@export var recovery_frames: int = 3         # Frames after hitbox disabled, before next action

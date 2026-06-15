## Receives hits from HitboxComponent. Delegates damage to HealthComponent.
## Per hit-detection GDD: i-frames prevent re-hit from same attack.
class_name HurtboxComponent
extends Area2D

var _health_component: HealthComponent = null
var _is_invincible: bool = false

signal damage_requested(damage: int, damage_type: int, is_crit: bool, knockback_direction: float)

func _ready() -> void:
	monitoring = true

## Initialize with a reference to the owning entity's HealthComponent.
func setup(health: HealthComponent) -> void:
	_health_component = health

## Apply damage from a hit. Returns true if damage was actually applied.
func apply_hit(damage: int, damage_type: int, is_crit: bool, knockback_direction: float = 1.0) -> bool:
	if _health_component and _health_component.is_invincible():
		return false
	if _is_invincible:
		return false
	damage_requested.emit(damage, damage_type, is_crit, knockback_direction)
	if _health_component:
		_health_component.damage(damage, damage_type, is_crit)
		return true
	return false

## Set invincibility state (e.g., during dodge i-frames).
func set_invincible(invincible: bool) -> void:
	_is_invincible = invincible

## Check if this hurtbox is currently invincible.
func is_invincible() -> bool:
	return _is_invincible or (_health_component and _health_component.is_invincible())

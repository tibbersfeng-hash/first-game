## Attack hitbox component. Emits signals when overlapping a HurtboxComponent.
## Managed by HitManager to prevent multi-hit.
## ADR-006: hitbox activation tied to attack phase (startup→active→recovery).
class_name HitboxComponent
extends Area2D

var _attack_data: AttackData = null
var _hit_targets: Array[HurtboxComponent] = []

signal hit_hurtbox(hurtbox: HurtboxComponent)

func _ready() -> void:
	# Disable by default — only enabled during active frames
	monitoring = false
	area_entered.connect(_on_area_entered)

## Activate this hitbox for the given attack data.
func activate(attack: AttackData) -> void:
	_attack_data = attack
	_hit_targets.clear()
	monitoring = true

## Deactivate this hitbox (end of active frames or after first hit).
func deactivate() -> void:
	monitoring = false
	_attack_data = null
	_hit_targets.clear()

## Check if this hitbox has already hit a specific hurtbox this attack.
func has_hit_target(hurtbox: HurtboxComponent) -> bool:
	return hurtbox in _hit_targets

## Record that this hitbox hit a specific hurtbox (prevents multi-hit).
func record_hit(hurtbox: HurtboxComponent) -> void:
	if not has_hit_target(hurtbox):
		_hit_targets.append(hurtbox)

## Get the attack data currently associated with this hitbox.
func get_attack_data() -> AttackData:
	return _attack_data

func _on_area_entered(area: Area2D) -> void:
	if area is HurtboxComponent:
		var hurtbox: HurtboxComponent = area as HurtboxComponent
		if not has_hit_target(hurtbox):
			hit_hurtbox.emit(hurtbox)

## Manages hit resolution between HitboxComponent and HurtboxComponent.
## Prevents same hitbox from hitting same hurtbox twice per attack.
## Triggers all feedback effects via SignalBus (ADR-003: cross-layer).
## Not an autoload — attached as a child of the attacker entity.
class_name HitManager
extends Node

## Optional callback to query current combo count from the attacker.
## Set by the gameplay layer (e.g., PlayerCombat) for damage scaling.
var combo_count_query: Callable = func() -> int: return 0

## Resolve a hit between a hitbox and hurtbox.
## Handles damage calculation, multi-hit prevention, and SignalBus feedback.
func resolve_hit(hitbox: HitboxComponent, hurtbox: HurtboxComponent, attacker_facing: int = 1) -> void:
	if hitbox.has_hit_target(hurtbox):
		return  # Already hit this target this attack

	var attack: AttackData = hitbox.get_attack_data()
	if attack == null:
		return

	# Calculate damage with combo scaling
	var combo_count: int = combo_count_query.call()
	var combo_multiplier: float = 1.0 + combo_count * CombatData.player_config.combo_damage_bonus_pct
	var is_crit: bool = randf() < CombatData.player_config.crit_chance
	var crit_multiplier: float = CombatData.player_config.crit_multiplier if is_crit else 1.0
	var final_damage: int = int(attack.damage * combo_multiplier * crit_multiplier)

	# Apply hit to hurtbox
	var hit_applied: bool = hurtbox.apply_hit(final_damage, attack.type, is_crit, float(attacker_facing))
	if not hit_applied:
		return  # Target invincible

	# Record the hit to prevent multi-hit
	hitbox.record_hit(hurtbox)

	# Cross-layer notifications via SignalBus (ADR-003)
	var target: Node2D = _get_hurtbox_owner(hurtbox)
	var target_pos: Vector2 = target.global_position + Vector2(0, -30) if target else Vector2.ZERO

	SignalBus.hit_confirmed.emit(target, final_damage, attack.type, is_crit)
	SignalBus.damage_number_requested.emit(target_pos, final_damage, attack.type, is_crit)

	# Screen shake
	if attack.shake_intensity > 0.0:
		SignalBus.screen_shake_requested.emit(attack.shake_intensity)

	# Enemy hit event
	SignalBus.enemy_hit.emit(target, final_damage)

func _get_hurtbox_owner(hurtbox: HurtboxComponent) -> Node2D:
	var owner_node: Node = hurtbox.owner
	if owner_node is Node2D:
		return owner_node as Node2D
	return null

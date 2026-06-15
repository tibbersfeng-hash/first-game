## Enemy hit-stun state — brief stun after taking damage.
## ADR-006: stun duration is frame-based.
class_name EnemyHitStunState
extends State

@export var stun_frames: int = 12  # ~200ms at 60fps

func enter() -> void:
	var enemy: CharacterBody2D = get_owner_entity()
	if enemy:
		enemy.sprite.color = Color.WHITE

func physics_update(delta: float) -> void:
	var enemy: CharacterBody2D = get_owner_entity()
	if not enemy:
		return

	# Apply gravity
	enemy.get_node("MovementComponent").apply_gravity(enemy, delta)

	# Friction to slow knockback
	var friction: float = enemy.enemy_data.friction if enemy.enemy_data else 300.0
	enemy.velocity.x = move_toward(enemy.velocity.x, 0, friction * delta)

	enemy.move_and_slide()

	# Flash back to normal color
	if frame_in_state == 3:
		if enemy._flash_tween:
			enemy._flash_tween.kill()
		enemy._flash_tween = enemy.create_tween()
		enemy._flash_tween.tween_property(enemy.sprite, "color", Color(0.9, 0.3, 0.3), 0.15)

	# Check if stun is over
	if frame_in_state >= stun_frames:
		transition_to("Idle")

## Enemy idle state — standing still, waiting.
## MVP: training dummy that just stands there.
class_name EnemyIdleState
extends State

func physics_update(delta: float) -> void:
	var enemy: CharacterBody2D = get_owner_entity()
	if not enemy:
		return

	# Apply gravity
	enemy.get_node("MovementComponent").apply_gravity(enemy, delta)

	# Friction to slow down knockback
	enemy.velocity.x = move_toward(enemy.velocity.x, 0, enemy.enemy_data.friction * delta if enemy.enemy_data else 300 * delta)

	enemy.move_and_slide()

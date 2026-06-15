## Player hit-stun state — brief stun after taking damage.
## ADR-006: stun duration is frame-based.
class_name PlayerHitStunState
extends State

@export var stun_frames: int = 15  # ~250ms at 60fps

func enter() -> void:
	var player: CharacterBody2D = get_owner_entity()
	if player:
		player.flash_sprite(Color(1.0, 0.5, 0.5))

func physics_update(delta: float) -> void:
	var player: CharacterBody2D = get_owner_entity()
	if not player:
		return

	# Apply gravity during stun
	player.get_node("MovementComponent").apply_gravity(player, delta)

	# Friction to slow down knockback
	player.velocity.x = move_toward(player.velocity.x, 0, 300 * delta)

	player.move_and_slide()

	# Check if stun is over
	if frame_in_state >= stun_frames:
		transition_to("Idle")

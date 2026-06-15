## Player jump state — airborne movement and gravity.
## Transitions to Idle on landing, Attack on attack input.
class_name PlayerJumpState
extends State

func enter() -> void:
	var player: CharacterBody2D = get_owner_entity()
	if player:
		player.get_node("MovementComponent").try_jump(player)

func physics_update(delta: float) -> void:
	var player: CharacterBody2D = get_owner_entity()
	if not player:
		return

	var movement: MovementComponent = player.get_node("MovementComponent")
	var input_buffer: InputBuffer = player.get_node("InputBuffer")

	# Apply gravity
	movement.apply_gravity(player, delta)

	# Air movement
	var input_dir: float = input_buffer.get_vector("move_left", "move_right")
	movement.apply_movement(player, input_dir)

	# Landing check
	if player.is_on_floor():
		movement.notify_landed()
		transition_to("Idle")
		return

	# Air attack (limited — only light attacks in air for MVP)
	if input_buffer.consume("attack_light"):
		if player.combo_manager.try_attack(CombatData.AttackType.LIGHT):
			transition_to("Attack")
			return

	player.move_and_slide()

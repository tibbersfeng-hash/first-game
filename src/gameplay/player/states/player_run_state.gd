## Player run state — horizontal movement.
## Transitions to Idle when no input, Jump when jump buffered, Attack on attack input.
class_name PlayerRunState
extends State

func physics_update(delta: float) -> void:
	var player: CharacterBody2D = get_owner_entity()
	if not player:
		return

	var movement: MovementComponent = player.get_node("MovementComponent")
	var input_buffer: InputBuffer = player.get_node("InputBuffer")

	# Apply gravity
	movement.apply_gravity(player, delta)

	# Check movement input
	var input_dir: float = input_buffer.get_vector("move_left", "move_right")
	if input_dir == 0.0:
		transition_to("Idle")
		return

	# Update facing
	player.facing = 1 if input_dir > 0 else -1
	player.visual_pivot.scale.x = player.facing

	# Apply movement
	movement.apply_movement(player, input_dir)

	# Check jump
	if input_buffer.consume("jump") and player.is_on_floor():
		transition_to("Jump")
		return

	# Check attack inputs
	if input_buffer.consume("attack_special") and player.energy.can_special():
		if player.combo_manager.try_attack(CombatData.AttackType.SPECIAL):
			transition_to("Attack")
			return
	if input_buffer.consume("attack_heavy") and player.energy.can_heavy():
		if player.combo_manager.try_attack(CombatData.AttackType.HEAVY):
			transition_to("Attack")
			return
	if input_buffer.consume("attack_light"):
		if player.combo_manager.try_attack(CombatData.AttackType.LIGHT):
			transition_to("Attack")
			return

	player.move_and_slide()

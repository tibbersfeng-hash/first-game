## Player idle state — standing still, waiting for input.
## Checks InputBuffer for transitions to Run, Jump, or Attack.
class_name PlayerIdleState
extends State

func enter() -> void:
	var player: CharacterBody2D = get_owner_entity()
	if player:
		player.velocity.x = 0

func physics_update(delta: float) -> void:
	var player: CharacterBody2D = get_owner_entity()
	if not player:
		return

	# Apply gravity
	player.get_node("MovementComponent").apply_gravity(player, delta)

	# Check movement input
	var input_dir: float = player.get_node("InputBuffer").get_vector("move_left", "move_right")
	if input_dir != 0.0:
		player.facing = 1 if input_dir > 0 else -1
		player.visual_pivot.scale.x = player.facing
		transition_to("Run")
		return

	# Check jump
	if player.get_node("InputBuffer").consume("jump"):
		transition_to("Jump")
		return

	# Check attack inputs (priority: special > heavy > light)
	if player.get_node("InputBuffer").consume("attack_special") and player.energy.can_special():
		if player.combo_manager.try_attack(CombatData.AttackType.SPECIAL):
			transition_to("Attack")
			return
	if player.get_node("InputBuffer").consume("attack_heavy") and player.energy.can_heavy():
		if player.combo_manager.try_attack(CombatData.AttackType.HEAVY):
			transition_to("Attack")
			return
	if player.get_node("InputBuffer").consume("attack_light"):
		if player.combo_manager.try_attack(CombatData.AttackType.LIGHT):
			transition_to("Attack")
			return

	player.move_and_slide()

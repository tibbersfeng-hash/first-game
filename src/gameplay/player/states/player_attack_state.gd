## Player attack state — frame-based attack phase management.
## ADR-006: uses frame_in_state to determine attack phase.
## Phases: startup → active → recovery.
## HitboxComponent is activated/deactivated based on current phase.
## During recovery, checks InputBuffer for combo continuation.
class_name PlayerAttackState
extends State

var _current_attack: AttackData = null

func enter() -> void:
	var player: CharacterBody2D = get_owner_entity()
	if not player:
		return

	# Get current attack data from ComboManager
	_current_attack = player.combo_manager.get_current_step_data()
	if _current_attack == null:
		transition_to("Idle")
		return

	# Spend energy if needed
	if _current_attack.energy_cost > 0:
		if not player.energy.spend(_current_attack.energy_cost):
			transition_to("Idle")
			return

	# Attack push (slight forward movement)
	player.get_node("MovementComponent").start_attack_push(player, player.facing)

	# Visual feedback
	match _current_attack.type:
		CombatData.AttackType.LIGHT:
			player.flash_sprite(Color(1.5, 1.5, 1.0))
		CombatData.AttackType.HEAVY:
			player.flash_sprite(Color(1.5, 1.0, 0.5))
			player.squash_stretch(Vector2(1.3, 0.7))
		CombatData.AttackType.SPECIAL:
			player.flash_sprite(Color(1.5, 0.5, 0.5))
			player.squash_stretch(Vector2(1.5, 0.5))

	# Emit attack started signal
	SignalBus.attack_started.emit(_current_attack.attack_id)

func physics_update(delta: float) -> void:
	var player: CharacterBody2D = get_owner_entity()
	if not player or _current_attack == null:
		transition_to("Idle")
		return

	var movement: MovementComponent = player.get_node("MovementComponent")
	var hitbox_comp: HitboxComponent = player.get_node("HitboxPivot/HitboxComponent")

	# Apply gravity
	movement.apply_gravity(player, delta)

	# Attack phase management based on frame_in_state (ADR-006)
	var frame: int = frame_in_state
	var startup_end: int = _current_attack.startup_frames
	var active_end: int = _current_attack.startup_frames + _current_attack.active_frames
	var total_frames: int = _current_attack.startup_frames + _current_attack.active_frames + _current_attack.recovery_frames

	if frame < startup_end:
		# ─── Startup phase ───
		hitbox_comp.deactivate()
	elif frame < active_end:
		# ─── Active phase — hitbox enabled ───
		if not hitbox_comp.monitoring:
			hitbox_comp.activate(_current_attack)
			# Position hitbox based on facing
			player.get_node("HitboxPivot").position = _current_attack.hitbox_offset * Vector2(player.facing, 1.0)
			player.get_node("HitboxPivot").scale.x = player.facing
	elif frame < total_frames:
		# ─── Recovery phase — hitbox disabled, check combo input ───
		hitbox_comp.deactivate()
		movement.stop_attack_push()

		# Check InputBuffer for combo continuation
		_check_combo_input()
	else:
		# ─── Attack complete ───
		hitbox_comp.deactivate()
		movement.stop_attack_push()
		transition_to("Idle")
		return

	player.move_and_slide()

func exit() -> void:
	var player: CharacterBody2D = get_owner_entity()
	if not player:
		return

	# Clean up hitbox
	var hitbox_comp: HitboxComponent = player.get_node("HitboxPivot/HitboxComponent")
	hitbox_comp.deactivate()

	# Stop attack push
	player.get_node("MovementComponent").stop_attack_push()

	_current_attack = null

## Check for combo input during recovery phase.
func _check_combo_input() -> void:
	var player: CharacterBody2D = get_owner_entity()
	if not player:
		return

	var input_buffer: InputBuffer = player.get_node("InputBuffer")

	# Try each attack type in priority order
	if input_buffer.consume("attack_special") and player.energy.can_special():
		if player.combo_manager.try_attack(CombatData.AttackType.SPECIAL):
			_start_next_combo_step(player)
			return
	if input_buffer.consume("attack_heavy") and player.energy.can_heavy():
		if player.combo_manager.try_attack(CombatData.AttackType.HEAVY):
			_start_next_combo_step(player)
			return
	if input_buffer.consume("attack_light"):
		if player.combo_manager.try_attack(CombatData.AttackType.LIGHT):
			_start_next_combo_step(player)
			return

## Start the next combo step by re-entering Attack state.
func _start_next_combo_step(player: CharacterBody2D) -> void:
	transition_to("Attack")

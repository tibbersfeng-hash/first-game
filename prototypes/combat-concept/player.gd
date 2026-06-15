# Player controller — Q版格斗角色
# Controls: A/D move, W jump, J light attack, K heavy attack, L special
# ADR-006: Frame-based attack phase management
# ADR-003: Cross-layer communication via SignalBus

extends CharacterBody2D

@onready var _combo_manager: ComboManager = $ComboManager
@onready var _hit_stop: HitStopManager = $HitStopManager
@onready var _visual_pivot: Node2D = $VisualPivot
@onready var _sprite: ColorRect = $VisualPivot/BodySprite
@onready var _hitbox: Area2D = $HitboxPivot/HitboxArea
@onready var _hitbox_shape: CollisionShape2D = $HitboxPivot/HitboxArea/CollisionShape2D
@onready var _hitbox_pivot: Node2D = $HitboxPivot
@onready var _combo_label: Label = $ComboLabel

var _is_attacking: bool = false
var _facing: int = 1  # 1=right, -1=left
var _combo_count: int = 0

# Frame-based attack state (ADR-006: replaces AttackTimer)
var _current_attack_data: AttackStepData = null
var _frame_in_attack: int = 0

# Tween references (stored to prevent conflicts)
var _flash_tween: Tween = null
var _squash_tween: Tween = null

func _physics_process(delta: float) -> void:
	# Don't process movement during hit-stop
	if _hit_stop.is_in_hit_stop():
		return

	# Gravity
	if not is_on_floor():
		velocity.y += CombatData.player_config.gravity * delta

	# Movement
	var input_dir: float = Input.get_axis("move_left", "move_right")
	if input_dir != 0 and not _is_attacking:
		_facing = 1 if input_dir > 0 else -1
		velocity.x = input_dir * CombatData.player_config.move_speed
		_visual_pivot.scale.x = _facing
		_hitbox_pivot.scale.x = _facing
	elif not _is_attacking:
		velocity.x = move_toward(velocity.x, 0, CombatData.player_config.move_speed * 2)

	# Jump
	if Input.is_action_just_pressed("jump") and is_on_floor() and not _is_attacking:
		velocity.y = CombatData.player_config.jump_velocity

	# Attack phase management (ADR-006: frame counting replaces Timer)
	if _is_attacking and _current_attack_data:
		_frame_in_attack += 1
		var step: AttackStepData = _current_attack_data
		var active_end: int = step.startup_frames + step.active_frames
		if _frame_in_attack <= step.startup_frames:
			# Startup phase - hitbox disabled
			_hitbox_shape.disabled = true
		elif _frame_in_attack <= active_end:
			# Active phase - hitbox enabled
			_hitbox_shape.disabled = false
			_hitbox_pivot.position = Vector2(_facing * 30, -20)
		elif _frame_in_attack > active_end + step.recovery_frames:
			# Attack complete
			_is_attacking = false
			_hitbox_shape.disabled = true
			_current_attack_data = null
		else:
			# Recovery phase - hitbox disabled, combo input allowed
			_hitbox_shape.disabled = true

	# Attack inputs
	if Input.is_action_just_pressed("attack_light"):
		_try_attack(CombatData.AttackType.LIGHT)
	elif Input.is_action_just_pressed("attack_heavy"):
		_try_attack(CombatData.AttackType.HEAVY)
	elif Input.is_action_just_pressed("attack_special"):
		_try_attack(CombatData.AttackType.SPECIAL)

	move_and_slide()

	# Combo count display
	if _combo_label:
		if _combo_count > 0:
			_combo_label.text = "COMBO x%d" % _combo_count
		else:
			_combo_label.text = ""

func _try_attack(attack_type: int) -> bool:
	if _is_attacking:
		if _current_attack_data == null:
			return false
		# Only allow combo input during recovery phase
		var active_end: int = _current_attack_data.startup_frames + _current_attack_data.active_frames
		if _frame_in_attack <= active_end:
			return false
	if _combo_manager.try_attack(attack_type):
		_is_attacking = true
		_frame_in_attack = 0
		velocity.x = _facing * CombatData.player_config.attack_push_speed
		return true
	return false

func _on_combo_step(step_index: int, step_data: AttackStepData) -> void:
	_combo_count += 1
	_current_attack_data = step_data
	_frame_in_attack = 0

	# Emit SignalBus for cross-layer consumers (ADR-003)
	SignalBus.combo_updated.emit(_combo_count, _combo_manager.get_current_chain_name())

	# Visual feedback (same layer, internal)
	match step_data.type:
		CombatData.AttackType.LIGHT:
			_flash_sprite(Color(1.5, 1.5, 1.0))
		CombatData.AttackType.HEAVY:
			_flash_sprite(Color(1.5, 1.0, 0.5))
			_squash_stretch(Vector2(1.3, 0.7))
		CombatData.AttackType.SPECIAL:
			_flash_sprite(Color(1.5, 0.5, 0.5))
			_squash_stretch(Vector2(1.5, 0.5))

func _on_combo_finished(chain_name: String) -> void:
	# Cross-layer: request screen shake via SignalBus (ADR-003)
	SignalBus.screen_shake_requested.emit(6.0)

func _on_combo_dropped() -> void:
	_combo_count = 0
	SignalBus.combo_dropped.emit()

func _on_hitbox_area_body_entered(body: Node2D) -> void:
	if body.has_method("take_damage"):
		var step_data: AttackStepData = _combo_manager.get_current_step_data()
		if step_data == null:
			return

		# Apply hit-stop (same layer, direct reference)
		_hit_stop.apply_hit_stop(step_data.hit_stop_frames)

		# Request screen shake via SignalBus (cross-layer, ADR-003)
		if step_data.shake_intensity > 0:
			SignalBus.screen_shake_requested.emit(step_data.shake_intensity)

		# Calculate damage (with combo multiplier)
		var damage: int = int(step_data.damage * (1.0 + _combo_count * CombatData.player_config.combo_damage_bonus_pct))
		var is_crit: bool = randf() < CombatData.player_config.crit_chance
		if is_crit:
			damage = int(damage * CombatData.player_config.crit_multiplier)

		# Deal damage (same layer, direct call with knockback direction)
		body.take_damage(damage, step_data.type, is_crit, _facing)

		# Request damage number via SignalBus (cross-layer, ADR-003)
		SignalBus.damage_number_requested.emit(body.global_position + Vector2(0, -30), damage, step_data.type, is_crit)

		# Notify other cross-layer listeners
		SignalBus.hit_confirmed.emit(body, damage, step_data.type, is_crit)

		# Squash on hit (same layer, internal)
		_squash_stretch(Vector2(0.8, 1.2))

		# Disable hitbox after first hit per swing (prevents multi-hit)
		_hitbox_shape.disabled = true

func _flash_sprite(color: Color) -> void:
	if _flash_tween:
		_flash_tween.kill()
	_sprite.color = color
	_flash_tween = create_tween()
	_flash_tween.tween_property(_sprite, "color", Color(0.4, 0.6, 1.0), 0.1)

func _squash_stretch(target_scale: Vector2) -> void:
	if _squash_tween:
		_squash_tween.kill()
	_squash_tween = create_tween()
	_squash_tween.tween_property(_sprite, "scale", target_scale, 0.05)
	_squash_tween.tween_property(_sprite, "scale", Vector2(1.0, 1.0), 0.1)

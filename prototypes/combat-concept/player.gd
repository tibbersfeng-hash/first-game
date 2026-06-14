# PROTOTYPE - NOT FOR PRODUCTION
# Player controller — Q版格斗角色
# Controls: A/D move, W jump, J light attack, K heavy attack, L special

extends CharacterBody2D

@onready var _combo_manager: ComboManager = $ComboManager
@onready var _hit_stop: HitStopManager = $HitStopManager
@onready var _sprite: ColorRect = $BodySprite
@onready var _hitbox: Area2D = $HitboxPivot/HitboxArea
@onready var _hitbox_shape: CollisionShape2D = $HitboxPivot/HitboxArea/CollisionShape2D
@onready var _hitbox_pivot: Node2D = $HitboxPivot
@onready var _attack_timer: Timer = $AttackTimer
@onready var _combo_label: Label = $ComboLabel

var _is_attacking: bool = false
var _facing: int = 1  # 1=right, -1=left
var _combo_count: int = 0

func _physics_process(delta: float) -> void:
	# Don't process movement during hit-stop
	if _hit_stop.is_in_hit_stop():
		return

	# Gravity
	if not is_on_floor():
		velocity.y += CombatData.GRAVITY * delta

	# Movement
	var input_dir: float = Input.get_axis("move_left", "move_right")
	if input_dir != 0 and not _is_attacking:
		_facing = 1 if input_dir > 0 else -1
		velocity.x = input_dir * CombatData.PLAYER_SPEED
		_sprite.scale.x = _facing
		_hitbox_pivot.scale.x = _facing
	elif not _is_attacking:
		velocity.x = move_toward(velocity.x, 0, CombatData.PLAYER_SPEED * 2)

	# Jump
	if Input.is_action_just_pressed("jump") and is_on_floor() and not _is_attacking:
		velocity.y = CombatData.JUMP_VELOCITY

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

func _try_attack(attack_type: int) -> void:
	if _is_attacking and not _combo_manager.get_current_step_data().is_empty():
		pass  # Allow combo input during attack

	if _combo_manager.try_attack(attack_type):
		_is_attacking = true
		velocity.x = _facing * 50  # slight forward push on attack
	else:
		return

func _on_combo_step(step_index: int, step_data: Dictionary) -> void:
	_combo_count += 1

	# Activate hitbox
	_hitbox_shape.disabled = false
	_hitbox_pivot.position = Vector2(_facing * 30, -20)

	# Attack duration based on type
	var duration: float = 0.15
	match step_data["type"]:
		CombatData.AttackType.LIGHT:
			duration = 0.12
			_flash_sprite(Color(1.5, 1.5, 1.0))
		CombatData.AttackType.HEAVY:
			duration = 0.25
			_flash_sprite(Color(1.5, 1.0, 0.5))
			_squash_stretch(Vector2(1.3, 0.7))
		CombatData.AttackType.SPECIAL:
			duration = 0.4
			_flash_sprite(Color(1.5, 0.5, 0.5))
			_squash_stretch(Vector2(1.5, 0.5))

	_attack_timer.wait_time = duration
	_attack_timer.start()

func _on_combo_finished(chain_name: String) -> void:
	# Big finish feedback
	var camera: Camera2D = get_viewport().get_camera_2d()
	if camera and camera.has_method("shake"):
		camera.call("shake", 6.0)

func _on_combo_dropped() -> void:
	_combo_count = 0

func _on_attack_timer_timeout() -> void:
	_is_attacking = false
	_hitbox_shape.disabled = true

func _on_hitbox_area_body_entered(body: Node2D) -> void:
	if body.has_method("take_damage"):
		var step_data: Dictionary = _combo_manager.get_current_step_data()
		if step_data.is_empty():
			return

		# Apply hit-stop
		_hit_stop.apply_hit_stop(step_data["hit_stop"])

		# Apply screen shake
		var camera: Camera2D = get_viewport().get_camera_2d()
		if camera and camera.has_method("shake"):
			camera.call("shake", step_data["shake"])

		# Calculate damage (with combo multiplier)
		var damage: int = int(step_data["damage"] * (1.0 + _combo_count * 0.1))
		var is_crit: bool = randf() < 0.15  # 15% crit chance
		if is_crit:
			damage = int(damage * 1.5)

		# Deal damage
		body.take_damage(damage, step_data["type"], is_crit)

		# Squash on hit
		_squash_stretch(Vector2(0.8, 1.2))

		# Disable hitbox after first hit per swing (prevents multi-hit)
		_hitbox_shape.disabled = true

func _flash_sprite(color: Color) -> void:
	_sprite.color = color
	var tween: Tween = create_tween()
	tween.tween_property(_sprite, "color", Color(0.4, 0.6, 1.0), 0.1)

func _squash_stretch(target_scale: Vector2) -> void:
	var tween: Tween = create_tween()
	tween.tween_property(_sprite, "scale", target_scale, 0.05)
	tween.tween_property(_sprite, "scale", Vector2(1.0, 1.0), 0.1)

# PROTOTYPE - NOT FOR PRODUCTION
# Dummy enemy — training dummy that takes hits and shows feedback
# Question: Does hitting a target with full juice feel satisfying?

extends CharacterBody2D

@onready var _sprite: ColorRect = $BodySprite
@onready var _health_bar: ProgressBar = $HealthBar
@onready var _state_label: Label = $StateLabel

var _max_health: int = 500
var _health: int = 500
var _is_hurt: bool = false
var _hit_count: int = 0

signal died

func _ready() -> void:
	_update_health_bar()
	_state_label.text = "DUMMY"

func take_damage(damage: int, attack_type: int, is_crit: bool) -> void:
	_health = maxi(_health - damage, 0)
	_hit_count += 1
	_update_health_bar()

	# Spawn damage number
	_spawn_damage_number(damage, attack_type, is_crit)

	# Hurt feedback — knockback + squash
	var knockback: float = 50.0
	if attack_type == CombatData.AttackType.HEAVY:
		knockback = 120.0
	elif attack_type == CombatData.AttackType.SPECIAL:
		knockback = 200.0
	velocity.x = -sign(global_position.x - get_viewport().get_camera_2d().global_position.x) * knockback

	# Squash on hit
	_squash_on_hit(attack_type)

	# Flash white
	_flash_hurt()

	# Update label
	_state_label.text = "HIT x%d" % _hit_count

	if _health <= 0:
		_die()

func _physics_process(delta: float) -> void:
	if not is_on_floor():
		velocity.y += CombatData.GRAVITY * delta

	velocity.x = move_toward(velocity.x, 0, 300 * delta)
	move_and_slide()

func _update_health_bar() -> void:
	if _health_bar:
		_health_bar.value = (float(_health) / float(_max_health)) * 100.0

func _spawn_damage_number(damage: int, attack_type: int, is_crit: bool) -> void:
	var dmg_scene: PackedScene = preload("res://damage_number.tscn")
	var dmg_node: Label = dmg_scene.instantiate()
	get_parent().add_child(dmg_node)
	dmg_node.spawn(global_position + Vector2(0, -30), damage, attack_type, is_crit)

func _squash_on_hit(attack_type: int) -> void:
	var squash: Vector2 = Vector2(0.8, 1.2)
	match attack_type:
		CombatData.AttackType.LIGHT:
			squash = Vector2(0.85, 1.15)
		CombatData.AttackType.HEAVY:
			squash = Vector2(0.7, 1.3)
		CombatData.AttackType.SPECIAL:
			squash = Vector2(0.5, 1.5)
	var tween: Tween = create_tween()
	tween.tween_property(_sprite, "scale", squash, 0.05)
	tween.tween_property(_sprite, "scale", Vector2(1.0, 1.0), 0.15)

func _flash_hurt() -> void:
	_sprite.color = Color.WHITE
	var tween: Tween = create_tween()
	tween.tween_property(_sprite, "color", Color(0.9, 0.3, 0.3), 0.15)

func _die() -> void:
	_state_label.text = "KO!"
	# Respawn after delay
	var tween: Tween = create_tween()
	tween.tween_property(_sprite, "modulate:a", 0.0, 0.3)
	tween.tween_callback(_respawn)

func _respawn() -> void:
	_health = _max_health
	_hit_count = 0
	_update_health_bar()
	_sprite.modulate.a = 1.0
	_sprite.color = Color(0.9, 0.3, 0.3)
	_state_label.text = "DUMMY"

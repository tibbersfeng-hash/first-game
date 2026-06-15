# Dummy enemy — training dummy that takes hits and shows feedback
# ADR-003: No camera references, knockback direction passed via parameter
# ADR-004: Uses EnemyConfig Resource from CombatData

extends CharacterBody2D

@onready var _visual_pivot: Node2D = $VisualPivot
@onready var _sprite: ColorRect = $VisualPivot/BodySprite
@onready var _health_bar: ProgressBar = $HealthBar
@onready var _state_label: Label = $StateLabel

var _health: int = 0
var _is_hurt: bool = false
var _hit_count: int = 0

# Frame-based respawn timer
var _respawn_timer_frames: int = 0

# Tween references (stored to prevent conflicts)
var _squash_tween: Tween = null
var _flash_tween: Tween = null
var _death_tween: Tween = null

func _ready() -> void:
	_health = CombatData.enemy_config.max_health
	_update_health_bar()
	_state_label.text = "DUMMY"

func take_damage(damage: int, attack_type: int, is_crit: bool, knockback_direction: float = 1.0) -> void:
	_health = maxi(_health - damage, 0)
	_hit_count += 1
	_update_health_bar()

	# Knockback (using passed direction instead of camera lookup)
	var knockback: float = CombatData.enemy_config.light_knockback
	match attack_type:
		CombatData.AttackType.HEAVY:
			knockback = CombatData.enemy_config.heavy_knockback
		CombatData.AttackType.SPECIAL:
			knockback = CombatData.enemy_config.special_knockback
	velocity.x = knockback_direction * knockback

	# Squash on hit
	_squash_on_hit(attack_type)

	# Flash white
	_flash_hurt()

	# Update label
	_state_label.text = "HIT x%d" % _hit_count

	if _health <= 0:
		_die()

func _physics_process(delta: float) -> void:
	# Handle respawn timer
	if _respawn_timer_frames > 0:
		_respawn_timer_frames -= 1
		if _respawn_timer_frames <= 0:
			_respawn()
		return

	if not is_on_floor():
		velocity.y += CombatData.enemy_config.gravity * delta

	velocity.x = move_toward(velocity.x, 0, CombatData.enemy_config.friction * delta)
	move_and_slide()

func _update_health_bar() -> void:
	if _health_bar:
		_health_bar.value = (float(_health) / float(CombatData.enemy_config.max_health)) * 100.0

func _squash_on_hit(attack_type: int) -> void:
	var squash: Vector2 = Vector2(0.85, 1.15)
	match attack_type:
		CombatData.AttackType.LIGHT:
			squash = Vector2(0.85, 1.15)
		CombatData.AttackType.HEAVY:
			squash = Vector2(0.7, 1.3)
		CombatData.AttackType.SPECIAL:
			squash = Vector2(0.5, 1.5)
	if _squash_tween:
		_squash_tween.kill()
	_squash_tween = create_tween()
	_squash_tween.tween_property(_sprite, "scale", squash, 0.05)
	_squash_tween.tween_property(_sprite, "scale", Vector2(1.0, 1.0), 0.15)

func _flash_hurt() -> void:
	_sprite.color = Color.WHITE
	if _flash_tween:
		_flash_tween.kill()
	_flash_tween = create_tween()
	_flash_tween.tween_property(_sprite, "color", Color(0.9, 0.3, 0.3), 0.15)

func _die() -> void:
	_state_label.text = "KO!"
	# Cross-layer: notify via SignalBus (ADR-003)
	SignalBus.enemy_died.emit(self)
	if _death_tween:
		_death_tween.kill()
	_death_tween = create_tween()
	_death_tween.tween_property(_sprite, "modulate:a", 0.0, 0.3)
	_death_tween.tween_callback(func(): _respawn_timer_frames = CombatData.enemy_config.respawn_frames)

func _respawn() -> void:
	_health = CombatData.enemy_config.max_health
	_hit_count = 0
	_update_health_bar()
	_sprite.modulate.a = 1.0
	_sprite.color = Color(0.9, 0.3, 0.3)
	_state_label.text = "DUMMY"

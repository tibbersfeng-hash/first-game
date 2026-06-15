## Base enemy — combines core systems for enemy entities.
## MVP: training dummy with hit-stun and respawn (similar to prototype).
## Configured from EnemyData Resource.
extends CharacterBody2D

@export var enemy_data_id: String = "goblin"

# ─── Core Systems ───
@onready var state_machine: StateMachine = $StateMachine
@onready var health: HealthComponent = $HealthComponent
@onready var hurtbox: HurtboxComponent = $HurtboxComponent
@onready var movement: MovementComponent = $MovementComponent

# ─── Visuals ───
@onready var visual_pivot: Node2D = $VisualPivot
@onready var sprite: ColorRect = $VisualPivot/BodySprite
@onready var health_bar: ProgressBar = $HealthBar
@onready var state_label: Label = $StateLabel

# ─── State ───
var enemy_data: EnemyData = null
var hit_count: int = 0

# ─── Tween refs ───
var _squash_tween: Tween = null
var _flash_tween: Tween = null
var _death_tween: Tween = null

func _ready() -> void:
	# Load enemy data from Resource
	enemy_data = CombatData.get_enemy_data(enemy_data_id)
	if enemy_data == null:
		push_warning("BaseEnemy: unknown enemy_data_id '%s'" % enemy_data_id)
		return

	# Initialize components from EnemyData
	health.max_hp = enemy_data.max_hp
	health.current_hp = enemy_data.max_hp
	movement.gravity = enemy_data.gravity
	movement.walk_speed = enemy_data.move_speed

	# Wire HurtboxComponent
	hurtbox.setup(health)

	# Wire health signals
	health.health_changed.connect(_on_health_changed)
	health.died.connect(_on_died)
	health.damage_taken.connect(_on_damage_taken)

	_update_health_bar()
	state_label.text = "ENEMY"

## Apply damage to this enemy. Called by HitManager via HurtboxComponent.
func take_damage(damage: int, damage_type: int, is_crit: bool, knockback_direction: float = 1.0) -> void:
	if health.is_invincible():
		return

	# Apply damage through HealthComponent
	health.damage(damage, damage_type, is_crit)

	# Knockback
	var knockback_speed: float = enemy_data.light_knockback
	match damage_type:
		CombatData.AttackType.HEAVY:
			knockback_speed = enemy_data.heavy_knockback
		CombatData.AttackType.SPECIAL:
			knockback_speed = enemy_data.special_knockback
	movement.apply_knockback(self, knockback_speed, knockback_direction)

	# Energy gain for the player (hit feedback)
	# This is handled by the player's HitManager via AttackData.energy_gain_on_hit

	# Transition to hit-stun
	hit_count += 1
	state_label.text = "HIT x%d" % hit_count
	if state_machine.get_current_state_name() != "dead":
		state_machine.transition_to("HitStun")

func _on_health_changed(current: int, maximum: int) -> void:
	_update_health_bar()

func _on_damage_taken(amount: int, damage_type: int, is_crit: bool) -> void:
	# Visual feedback
	_squash_on_hit(damage_type)
	_flash_hurt()

func _on_died() -> void:
	state_label.text = "KO!"
	SignalBus.enemy_died.emit(self)
	state_machine.transition_to("Dead")

func _update_health_bar() -> void:
	if health_bar:
		health_bar.value = health.get_health_ratio() * 100.0

func _squash_on_hit(damage_type: int) -> void:
	var squash: Vector2 = Vector2(0.85, 1.15)
	match damage_type:
		CombatData.AttackType.HEAVY:
			squash = Vector2(0.7, 1.3)
		CombatData.AttackType.SPECIAL:
			squash = Vector2(0.5, 1.5)
	if _squash_tween:
		_squash_tween.kill()
	_squash_tween = create_tween()
	_squash_tween.tween_property(sprite, "scale", squash, 0.05)
	_squash_tween.tween_property(sprite, "scale", Vector2(1.0, 1.0), 0.15)

func _flash_hurt() -> void:
	sprite.color = Color.WHITE
	if _flash_tween:
		_flash_tween.kill()
	_flash_tween = create_tween()
	_flash_tween.tween_property(sprite, "color", Color(0.9, 0.3, 0.3), 0.15)

func _respawn() -> void:
	health.reset()
	hit_count = 0
	_update_health_bar()
	sprite.modulate.a = 1.0
	sprite.color = Color(0.9, 0.3, 0.3)
	state_label.text = "ENEMY"
	state_machine.transition_to("Idle")

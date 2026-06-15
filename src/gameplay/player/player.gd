## Player controller — Q版格斗角色
## Combines core systems: StateMachine, InputBuffer, Movement, Health, Energy, Combat.
## ADR-006: Frame-based timing. ADR-003: SignalBus for cross-layer communication.
extends CharacterBody2D

# ─── Core Systems ───
@onready var state_machine: StateMachine = $StateMachine
@onready var input_buffer: InputBuffer = $InputBuffer
@onready var movement: MovementComponent = $MovementComponent
@onready var health: HealthComponent = $HealthComponent
@onready var energy: EnergyComponent = $EnergyComponent
@onready var combo_manager: ComboManager = $ComboManager
@onready var hit_stop: HitStopManager = $HitStopManager
@onready var hit_manager: HitManager = $HitManager
@onready var hitbox: HitboxComponent = $HitboxPivot/HitboxComponent

# ─── Visuals ───
@onready var visual_pivot: Node2D = $VisualPivot
@onready var sprite: ColorRect = $VisualPivot/BodySprite
@onready var combo_label: Label = $ComboLabel

# ─── State ───
var facing: int = 1  # 1=right, -1=left
var combo_count: int = 0

# ─── Tween refs ───
var _flash_tween: Tween = null
var _squash_tween: Tween = null

func _ready() -> void:
	# Initialize components from Resource config
	var config: PlayerConfig = CombatData.player_config
	movement.walk_speed = config.move_speed
	movement.jump_velocity = config.jump_velocity
	movement.gravity = config.gravity
	movement.attack_push_speed = config.attack_push_speed

	var energy_cfg: EnergyConfig = CombatData.energy_config
	energy.max_energy = energy_cfg.max_energy
	energy.starting_energy = energy_cfg.starting_energy
	energy.special_threshold = energy_cfg.special_threshold
	energy.dodge_cost = energy_cfg.dodge_cost
	energy.heavy_cost = energy_cfg.heavy_cost

	# Wire HitManager combo query
	hit_manager.combo_count_query = combo_manager.get_combo_count

	# Wire combo signals → SignalBus (cross-layer bridge, ADR-003)
	combo_manager.combo_step.connect(_on_combo_step)
	combo_manager.combo_finished.connect(_on_combo_finished)
	combo_manager.combo_dropped.connect(_on_combo_dropped)

	# Wire HitboxComponent → HitManager
	hitbox.hit_hurtbox.connect(_on_hitbox_hit_hurtbox)

	# Wire HealthComponent → SignalBus
	health.health_changed.connect(func(c, m): SignalBus.player_health_changed.emit(c, m))
	health.died.connect(func(): SignalBus.player_died.emit())

	# Wire EnergyComponent → SignalBus
	energy.energy_changed.connect(func(c, m): SignalBus.player_energy_changed.emit(c, m))

func _physics_process(delta: float) -> void:
	# Don't process movement during hit-stop
	if hit_stop.is_in_hit_stop():
		return
	# StateMachine.physics_update is called automatically by its own _physics_process
	# Movement gravity is applied by the current state

## Flash the sprite a color, then fade back.
func flash_sprite(color: Color) -> void:
	if _flash_tween:
		_flash_tween.kill()
	sprite.color = color
	_flash_tween = create_tween()
	_flash_tween.tween_property(sprite, "color", Color(0.4, 0.6, 1.0), 0.1)

## Squash and stretch the sprite.
func squash_stretch(target_scale: Vector2) -> void:
	if _squash_tween:
		_squash_tween.kill()
	_squash_tween = create_tween()
	_squash_tween.tween_property(sprite, "scale", target_scale, 0.05)
	_squash_tween.tween_property(sprite, "scale", Vector2(1.0, 1.0), 0.1)

# ─── Signal Handlers ───

func _on_combo_step(_step_index: int, step_data: AttackData) -> void:
	combo_count += 1
	SignalBus.combo_updated.emit(combo_count, combo_manager.get_current_chain_name())
	# Energy gain on hit will be handled when hit connects, not on step start

func _on_combo_finished(chain_name: String) -> void:
	SignalBus.screen_shake_requested.emit(6.0)

func _on_combo_dropped() -> void:
	combo_count = 0
	SignalBus.combo_dropped.emit()

func _on_hitbox_hit_hurtbox(hurtbox: HurtboxComponent) -> void:
	hit_manager.resolve_hit(hitbox, hurtbox, facing)

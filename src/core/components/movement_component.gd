## Reusable movement component for CharacterBody2D entities.
## Per player-controller GDD: instant acceleration, no inertia.
## Per ADR-004: all values data-driven from config.
## Operates ON a body passed as parameter (composition, not inheritance).
class_name MovementComponent
extends Node

@export var walk_speed: float = 300.0
@export var gravity: float = 980.0
@export var jump_velocity: float = -500.0
@export var air_control_ratio: float = 0.7
@export var attack_push_speed: float = 50.0
@export var knockback_friction: float = 600.0
@export var coyote_time_frames: int = 5   # ~83ms at 60fps
@export var jump_buffer_frames: int = 9    # ~150ms at 60fps

var _coyote_timer: FrameTimer = null
var _jump_buffer_timer: FrameTimer = null
var _is_attack_pushing: bool = false

signal jump_requested
signal landed

func _ready() -> void:
	_coyote_timer = FrameTimer.new()
	_jump_buffer_timer = FrameTimer.new()

func _physics_process(_delta: float) -> void:
	_coyote_timer.tick()
	_jump_buffer_timer.tick()

## Apply horizontal movement based on input direction.
## Instant acceleration (arcade feel) per player-controller GDD.
func apply_movement(body: CharacterBody2D, input_dir: float, is_attacking: bool = false) -> void:
	if is_attacking:
		if not _is_attack_pushing:
			body.velocity.x = move_toward(body.velocity.x, 0, knockback_friction * (1.0 / 60.0))
	elif input_dir != 0.0:
		var speed: float = walk_speed
		if not body.is_on_floor():
			speed = walk_speed * air_control_ratio
		body.velocity.x = input_dir * speed
	else:
		body.velocity.x = move_toward(body.velocity.x, 0, walk_speed * 2.0)

## Apply gravity when airborne.
func apply_gravity(body: CharacterBody2D, delta: float) -> void:
	if not body.is_on_floor():
		body.velocity.y += gravity * delta

## Attempt a jump. Returns true if jump was executed.
## Supports coyote time: can jump shortly after leaving a ledge.
func try_jump(body: CharacterBody2D) -> bool:
	if body.is_on_floor() or _coyote_timer.is_running():
		body.velocity.y = jump_velocity
		_coyote_timer.stop()
		_jump_buffer_timer.stop()
		jump_requested.emit()
		return true
	return false

## Record a jump input for buffering (called when jump pressed while airborne).
func buffer_jump() -> void:
	_jump_buffer_timer.start(jump_buffer_frames)

## Check if a buffered jump should execute (called when landing).
func check_jump_buffer(body: CharacterBody2D) -> bool:
	if _jump_buffer_timer.is_running():
		return try_jump(body)
	return false

## Notify that the entity just left the floor (for coyote time).
func notify_left_floor() -> void:
	_coyote_timer.start(coyote_time_frames)

## Notify that the entity just landed.
func notify_landed() -> void:
	_coyote_timer.stop()
	landed.emit()

## Apply attack push (slight forward movement during attack).
func start_attack_push(body: CharacterBody2D, facing: int) -> void:
	body.velocity.x = facing * attack_push_speed
	_is_attack_pushing = true

## Stop attack push (call when attack ends).
func stop_attack_push() -> void:
	_is_attack_pushing = false

## Apply knockback impulse.
func apply_knockback(body: CharacterBody2D, speed: float, direction: float) -> void:
	body.velocity.x = direction * speed

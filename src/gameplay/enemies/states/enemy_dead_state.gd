## Enemy dead state — fade out and respawn after delay.
## ADR-006: respawn timer is frame-based.
class_name EnemyDeadState
extends State

var _respawn_timer: FrameTimer = null

func enter() -> void:
	var enemy: CharacterBody2D = get_owner_entity()
	if not enemy:
		return

	# Fade out sprite
	if enemy._death_tween:
		enemy._death_tween.kill()
	enemy._death_tween = enemy.create_tween()
	enemy._death_tween.tween_property(enemy.sprite, "modulate:a", 0.0, 0.3)

	# Start respawn timer
	var respawn_frames: int = enemy.enemy_data.respawn_frames if enemy.enemy_data else 18
	_respawn_timer = FrameTimer.new()
	_respawn_timer.start(respawn_frames)
	_respawn_timer.expired.connect(_on_respawn_timer_expired)

func physics_update(_delta: float) -> void:
	if _respawn_timer:
		_respawn_timer.tick()

func _on_respawn_timer_expired() -> void:
	var enemy: CharacterBody2D = get_owner_entity()
	if enemy:
		enemy._respawn()

# Damage number — floating damage text with color coding
# ADR-006: Frame-based lifetime, runs in _physics_process
# ADR-004: Reads from DamageNumberConfig Resource
# Pooled by DamageNumberPool — uses _recycle() instead of queue_free()

extends Label

var _velocity: Vector2 = Vector2.ZERO
var _lifetime_frames: int = 0
var _total_lifetime_frames: int = 0

func spawn(pos: Vector2, damage: int, damage_type: int, is_crit: bool = false) -> void:
	var config: DamageNumberConfig = CombatData.damage_number_config
	global_position = pos + Vector2(randf_range(-config.spread, config.spread), 0)
	text = str(damage)
	_velocity = Vector2(0, config.rise_speed)
	_lifetime_frames = config.lifetime_frames
	_total_lifetime_frames = config.lifetime_frames

	# Style by attack type
	horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	var font_size: int = config.font_size
	var color: Color = config.color_light

	match damage_type:
		CombatData.AttackType.LIGHT:
			color = config.color_light
		CombatData.AttackType.HEAVY:
			color = config.color_heavy
			font_size = int(config.font_size * 1.3)
		CombatData.AttackType.SPECIAL:
			color = config.color_special
			font_size = config.crit_font_size

	if is_crit:
		color = config.color_crit
		font_size = config.crit_font_size
		text = str(damage) + "!"

	add_theme_font_size_override("font_size", font_size)
	add_theme_color_override("font_color", color)

	# Outline for readability
	add_theme_color_override("font_outline_color", Color.BLACK)
	add_theme_constant_override("outline_size", config.outline_size)

	visible = true
	set_physics_process(true)

func _physics_process(_delta: float) -> void:
	_lifetime_frames -= 1
	if _lifetime_frames <= 0:
		_recycle()
		return

	var physics_delta: float = 1.0 / 60.0
	global_position += _velocity * physics_delta
	_velocity.y *= 0.97  # slow down rise

	# Fade out in last 30% of lifetime
	var fade_threshold: int = int(_total_lifetime_frames * 0.3)
	if fade_threshold > 0 and _lifetime_frames < fade_threshold:
		modulate.a = float(_lifetime_frames) / float(fade_threshold)

func _recycle() -> void:
	visible = false
	set_physics_process(false)
	modulate.a = 1.0
	text = "0"
	_velocity = Vector2.ZERO

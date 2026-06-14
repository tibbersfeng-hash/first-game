# PROTOTYPE - NOT FOR PROFTWARE
# Damage number — floating damage text with color coding
# Question: Do damage numbers with color scaling create satisfying feedback?

extends Label

var _velocity: Vector2 = Vector2.ZERO
var _lifetime: float = 0.0

func spawn(pos: Vector2, damage: int, attack_type: int, is_crit: bool = false) -> void:
	position = pos + Vector2(randf_range(-CombatData.DMG_NUMBER_SPREAD, CombatData.DMG_NUMBER_SPREAD), 0)
	text = str(damage)
	_velocity = Vector2(0, CombatData.DMG_NUMBER_RISE_SPEED)
	_lifetime = CombatData.DMG_NUMBER_LIFETIME

	# Style by attack type
	horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	var font_size: int = CombatData.DMG_NUMBER_FONT_SIZE
	var color: Color = CombatData.COLOR_LIGHT

	match attack_type:
		CombatData.AttackType.LIGHT:
			color = CombatData.COLOR_LIGHT
		CombatData.AttackType.HEAVY:
			color = CombatData.COLOR_HEAVY
			font_size = int(CombatData.DMG_NUMBER_FONT_SIZE * 1.3)
		CombatData.AttackType.SPECIAL:
			color = CombatData.COLOR_SPECIAL
			font_size = CombatData.DMG_NUMBER_CRIT_FONT_SIZE

	if is_crit:
		color = CombatData.COLOR_CRIT
		font_size = CombatData.DMG_NUMBER_CRIT_FONT_SIZE
		text = str(damage) + "!"

	add_theme_font_size_override("font_size", font_size)
	add_theme_color_override("font_color", color)

	# Outline for readability
	add_theme_color_override("font_outline_color", Color.BLACK)
	add_theme_constant_override("outline_size", 3)

func _process(delta: float) -> void:
	_lifetime -= delta
	if _lifetime <= 0:
		queue_free()
		return

	position += _velocity * delta
	_velocity.y *= 0.97  # slow down rise

	# Fade out in last 30% of lifetime
	var fade_threshold: float = CombatData.DMG_NUMBER_LIFETIME * 0.3
	if _lifetime < fade_threshold:
		modulate.a = _lifetime / fade_threshold

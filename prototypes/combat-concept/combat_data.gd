# PROTOTYPE - NOT FOR PRODUCTION
# Combat data config for 格斗萌主 prototype
# Question: Can simplified combos create satisfying Q-version combat feel?

class_name CombatData

# Combo definitions: sequence of input types
# Each combo step defines: input_type, damage_multiplier, hit_stop_frames, shake_intensity, anim_name
enum AttackType { LIGHT, HEAVY, SPECIAL }

const COMBO_CHAINS: Dictionary = {
	"basic": [
		{ "type": AttackType.LIGHT, "damage": 10, "hit_stop": 3, "shake": 0.0, "anim": "light_1", "window": 0.3 },
		{ "type": AttackType.LIGHT, "damage": 12, "hit_stop": 4, "shake": 1.0, "anim": "light_2", "window": 0.25 },
		{ "type": AttackType.HEAVY, "damage": 25, "hit_stop": 8, "shake": 4.0, "anim": "heavy", "window": 0.4 },
		{ "type": AttackType.SPECIAL, "damage": 50, "hit_stop": 12, "shake": 8.0, "anim": "special", "window": 0.0 },
	],
	"quick": [
		{ "type": AttackType.LIGHT, "damage": 8, "hit_stop": 2, "shake": 0.0, "anim": "light_1", "window": 0.2 },
		{ "type": AttackType.LIGHT, "damage": 8, "hit_stop": 2, "shake": 0.0, "anim": "light_2", "window": 0.2 },
		{ "type": AttackType.LIGHT, "damage": 15, "hit_stop": 5, "shake": 2.0, "anim": "light_3", "window": 0.0 },
	],
	"power": [
		{ "type": AttackType.HEAVY, "damage": 30, "hit_stop": 6, "shake": 3.0, "anim": "heavy", "window": 0.35 },
		{ "type": AttackType.SPECIAL, "damage": 60, "hit_stop": 15, "shake": 10.0, "anim": "special", "window": 0.0 },
	],
}

const PLAYER_SPEED: float = 300.0
const JUMP_VELOCITY: float = -500.0
const GRAVITY: float = 980.0
const COMBO_TIMEOUT: float = 0.8  # seconds before combo resets
const HIT_STOP_SCALE: float = 0.0  # time scale during hit-stop

# Damage number config
const DMG_NUMBER_FONT_SIZE: int = 32
const DMG_NUMBER_CRIT_FONT_SIZE: int = 48
const DMG_NUMBER_LIFETIME: float = 0.8
const DMG_NUMBER_RISE_SPEED: float = -80.0
const DMG_NUMBER_SPREAD: float = 30.0

# Colors for damage numbers
const COLOR_LIGHT: Color = Color.WHITE
const COLOR_HEAVY: Color = Color.YELLOW
const COLOR_SPECIAL: Color = Color(1.0, 0.3, 0.3)  # red
const COLOR_CRIT: Color = Color(1.0, 0.84, 0.0)  # gold

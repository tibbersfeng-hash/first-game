## Read-only combat configuration autoload (ADR-002).
## All data loaded from .tres Resources via preload (ADR-004).
## No runtime modification allowed.
## Autoload load order: CombatData first (ADR-002).
class_name CombatData
extends Node

enum AttackType { LIGHT, HEAVY, SPECIAL }

# Preloaded configs (read-only at runtime per ADR-004)
var player_config: PlayerConfig = preload("res://src/infrastructure/data/player_config.tres")
var damage_number_config: DamageNumberConfig = preload("res://src/infrastructure/data/damage_number_config.tres")
var energy_config: EnergyConfig = preload("res://src/infrastructure/data/energy_config.tres")

# Data indexed by ID/name, loaded in _ready
var combo_chains: Dictionary = {}  # String -> ComboData
var enemy_data: Dictionary = {}    # String -> EnemyData

func _ready() -> void:
	_load_combo_chains()
	_load_enemy_data()

func get_combo_chain(chain_name: String) -> ComboData:
	return combo_chains.get(chain_name, null) as ComboData

func get_enemy_data(enemy_id: String) -> EnemyData:
	return enemy_data.get(enemy_id, null) as EnemyData

## Determine the attack phase based on frame_in_state and attack data.
## ADR-006: phases are defined by integer frame boundaries.
func get_attack_phase(attack: AttackData, frame: int) -> String:
	if frame < attack.startup_frames:
		return "startup"
	elif frame < attack.startup_frames + attack.active_frames:
		return "active"
	else:
		return "recovery"

## Check if the current frame is within the cancel window.
func is_in_cancel_window(attack: AttackData, frame: int) -> bool:
	var recovery_start: int = attack.startup_frames + attack.active_frames
	if frame < recovery_start:
		return false
	var recovery_elapsed: int = frame - recovery_start
	var cancel_window: int = int(attack.recovery_frames * 0.5)
	return recovery_elapsed < cancel_window

func _load_combo_chains() -> void:
	var chains: Array[ComboData] = [
		preload("res://src/infrastructure/data/combos/basic_combo.tres"),
		preload("res://src/infrastructure/data/combos/quick_combo.tres"),
		preload("res://src/infrastructure/data/combos/power_combo.tres"),
	]
	for chain: ComboData in chains:
		combo_chains[chain.chain_name] = chain

func _load_enemy_data() -> void:
	var enemies: Array[EnemyData] = [
		preload("res://src/infrastructure/data/enemies/goblin.tres"),
		preload("res://src/infrastructure/data/enemies/goblin_elite.tres"),
	]
	for data: EnemyData in enemies:
		enemy_data[data.enemy_id] = data

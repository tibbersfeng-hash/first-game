# Combat data autoload — loads all configuration from Resource files
# ADR-004: All data is read-only at runtime, loaded from .tres files
# ADR-002: CombatData is the first autoload, provides O(1) read access

class_name CombatData
extends Node

# Attack type enum — used across multiple systems
enum AttackType { LIGHT, HEAVY, SPECIAL }

# Preloaded configs (read-only at runtime per ADR-004)
var player_config: PlayerConfig = preload("res://data/player_config.tres")
var enemy_config: EnemyConfig = preload("res://data/enemy_config.tres")
var damage_number_config: DamageNumberConfig = preload("res://data/damage_number_config.tres")
var combat_config: CombatConfig = preload("res://data/combat_config.tres")

# Combo chains indexed by chain_name, loaded from .tres
var combo_chains: Dictionary = {}

func _ready() -> void:
	var chains: Array[ComboChainData] = [
		preload("res://data/basic_combo.tres"),
		preload("res://data/quick_combo.tres"),
		preload("res://data/power_combo.tres"),
	]
	for chain: ComboChainData in chains:
		combo_chains[chain.chain_name] = chain

func get_combo_chain(chain_name: String) -> ComboChainData:
	return combo_chains.get(chain_name, null) as ComboChainData

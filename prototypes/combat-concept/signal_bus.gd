# SignalBus autoload — cross-layer event communication
# ADR-003: Stateless, only declares signals. No game state held here.
# Signal naming convention: player_, combo_, enemy_, hit_, damage_, screen_shake_

class_name SignalBus
extends Node

# ─── Combat events ───
signal hit_confirmed(target: Node2D, damage: int, damage_type: int, is_crit: bool)
signal damage_number_requested(position: Vector2, damage: int, damage_type: int, is_crit: bool)
signal screen_shake_requested(intensity: float)

# ─── Combo events ───
signal combo_updated(count: int, chain_name: String)
signal combo_dropped

# ─── Enemy events ───
signal enemy_died(enemy: CharacterBody2D)

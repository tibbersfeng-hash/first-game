## Cross-layer event bus (ADR-003).
## Stateless: only declares and forwards signals. No game state.
## Signal naming convention: player_, combo_, enemy_, hit_/attack_/damage_, room_/dungeon_, game_
## All signals have 4 or fewer parameters per ADR-003 constraint.
class_name SignalBus
extends Node

# ─── Player state ───
signal player_health_changed(current: int, maximum: int)
signal player_energy_changed(current: int, maximum: int)
signal player_died

# ─── Combat events ───
signal hit_confirmed(target: Node2D, damage: int, damage_type: int, is_crit: bool)
signal damage_number_requested(position: Vector2, damage: int, damage_type: int, is_crit: bool)
signal attack_started(attack_id: String)
signal attack_hit(attack_id: String, target: Node2D)

# ─── Combo events ───
signal combo_updated(count: int, chain_name: String)
signal combo_dropped
signal combo_maxed

# ─── Enemy events ───
signal enemy_died(enemy: CharacterBody2D)
signal enemy_hit(enemy: CharacterBody2D, damage: int)

# ─── Screen effects ───
signal screen_shake_requested(intensity: float)

# ─── Dungeon flow ───
signal room_entered(room_id: String)
signal room_cleared(rating: String)
signal dungeon_cleared(dungeon_id: String, stats: Dictionary)
signal dungeon_failed(reason: String)

# ─── Game state ───
signal game_paused
signal game_resumed
signal game_over

## Game state management autoload (ADR-002).
## Sole writer of game state. Other systems notify via signals.
## GameManager does NOT reference other Autoloads directly (ADR-002).
extends Node

signal game_state_changed(new_state: String)

var current_dungeon_id: String = ""
var current_room_index: int = 0
var revive_coins_remaining: int = 0
var game_state: String = "menu"  # menu | playing | paused | result

func start_dungeon(dungeon_id: String) -> void:
	current_dungeon_id = dungeon_id
	current_room_index = 0
	game_state = "playing"
	game_state_changed.emit(game_state)
	SignalBus.room_entered.emit("room_0")

func advance_room() -> void:
	current_room_index += 1
	SignalBus.room_entered.emit("room_%d" % current_room_index)

func use_revive_coin() -> bool:
	if revive_coins_remaining <= 0:
		return false
	revive_coins_remaining -= 1
	return true

func pause_game() -> void:
	if game_state != "playing":
		return
	game_state = "paused"
	get_tree().paused = true
	game_state_changed.emit(game_state)
	SignalBus.game_paused.emit()

func resume_game() -> void:
	if game_state != "paused":
		return
	game_state = "playing"
	get_tree().paused = false
	game_state_changed.emit(game_state)
	SignalBus.game_resumed.emit()

## Reset all state between sessions (ADR-002 validation criterion).
func reset() -> void:
	current_dungeon_id = ""
	current_room_index = 0
	game_state = "menu"
	get_tree().paused = false
	game_state_changed.emit(game_state)

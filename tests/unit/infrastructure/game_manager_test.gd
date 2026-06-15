extends "res://addons/gut/test.gd"

var gm: GameManager

func before_each() -> void:
	gm = GameManager.new()
	add_child(gm)

func after_each() -> void:
	gm.queue_free()

func test_initial_state() -> void:
	assert_eq(gm.game_state, "menu", "Initial game state should be menu")
	assert_eq(gm.current_dungeon_id, "", "Initial dungeon ID should be empty")
	assert_eq(gm.current_room_index, 0, "Initial room index should be 0")
	assert_eq(gm.revive_coins_remaining, 0, "Initial revive coins should be 0")

func test_start_dungeon() -> void:
	gm.start_dungeon("forest_dungeon")
	assert_eq(gm.game_state, "playing", "State should be playing after start_dungeon")
	assert_eq(gm.current_dungeon_id, "forest_dungeon", "Dungeon ID should be set")
	assert_eq(gm.current_room_index, 0, "Room index should be 0 at start")

func test_start_dungeon_signal() -> void:
	var new_state: String = ""
	gm.game_state_changed.connect(func(s): new_state = s)
	gm.start_dungeon("forest_dungeon")
	assert_eq(new_state, "playing", "game_state_changed should fire with 'playing'")

func test_advance_room() -> void:
	gm.start_dungeon("forest_dungeon")
	gm.advance_room()
	assert_eq(gm.current_room_index, 1, "Room index should increment")
	gm.advance_room()
	assert_eq(gm.current_room_index, 2, "Room index should increment again")

func test_use_revive_coin_success() -> void:
	gm.revive_coins_remaining = 3
	var result: bool = gm.use_revive_coin()
	assert_true(result, "use_revive_coin should succeed with coins available")
	assert_eq(gm.revive_coins_remaining, 2, "Revive coins should decrement")

func test_use_revive_coin_failure() -> void:
	gm.revive_coins_remaining = 0
	var result: bool = gm.use_revive_coin()
	assert_false(result, "use_revive_coin should fail with no coins")

func test_pause_game() -> void:
	gm.start_dungeon("forest_dungeon")
	gm.pause_game()
	assert_eq(gm.game_state, "paused", "State should be paused after pause_game")

func test_pause_game_not_playing() -> void:
	# Default state is "menu", not "playing"
	gm.pause_game()
	assert_eq(gm.game_state, "menu", "Should not pause when not playing")

func test_resume_game() -> void:
	gm.start_dungeon("forest_dungeon")
	gm.pause_game()
	gm.resume_game()
	assert_eq(gm.game_state, "playing", "State should be playing after resume")

func test_resume_game_not_paused() -> void:
	gm.start_dungeon("forest_dungeon")
	# Not paused, resume should be no-op
	gm.resume_game()
	assert_eq(gm.game_state, "playing", "Should remain playing when resume called without pause")

func test_reset() -> void:
	gm.start_dungeon("forest_dungeon")
	gm.advance_room()
	gm.advance_room()
	gm.reset()
	assert_eq(gm.game_state, "menu", "State should be menu after reset")
	assert_eq(gm.current_dungeon_id, "", "Dungeon ID should be cleared after reset")
	assert_eq(gm.current_room_index, 0, "Room index should be 0 after reset")

extends "res://addons/gut/test.gd"

func before_each() -> void:
	pass

func after_each() -> void:
	pass

func test_age_calculation() -> void:
	var input: BufferedInput = BufferedInput.new("attack_light", 10, 40)
	assert_eq(input.age(15), 5, "Age should be 5 when current=15 and recorded=10")

func test_age_zero() -> void:
	var input: BufferedInput = BufferedInput.new("attack_light", 10, 40)
	assert_eq(input.age(10), 0, "Age should be 0 at same frame")

func test_is_within_window_true() -> void:
	var input: BufferedInput = BufferedInput.new("attack_light", 10, 40)
	assert_true(input.is_within_window(13, 8), "Age 3 should be within window 8")

func test_is_within_window_boundary() -> void:
	var input: BufferedInput = BufferedInput.new("attack_light", 10, 40)
	assert_true(input.is_within_window(18, 8), "Age 8 should be within window 8 (inclusive)")

func test_is_within_window_expired() -> void:
	var input: BufferedInput = BufferedInput.new("attack_light", 10, 40)
	assert_false(input.is_within_window(19, 8), "Age 9 should not be within window 8")

func test_is_within_window_consumed() -> void:
	var input: BufferedInput = BufferedInput.new("attack_light", 10, 40)
	input.consumed = true
	assert_false(input.is_within_window(11, 8), "Consumed input should not be within window")

func test_default_values() -> void:
	var input: BufferedInput = BufferedInput.new()
	assert_eq(input.action_name, "", "Default action_name should be empty")
	assert_eq(input.priority, 0, "Default priority should be 0")
	assert_false(input.consumed, "Default consumed should be false")

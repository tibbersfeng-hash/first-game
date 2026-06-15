extends "res://addons/gut/test.gd"

var buffer: InputBuffer

func before_each() -> void:
	buffer = InputBuffer.new()
	add_child(buffer)
	# Simulate a few frames to initialize _current_frame
	for _i in range(2):
		buffer._physics_process(1.0 / 60.0)

func after_each() -> void:
	buffer.queue_free()

func test_record_and_consume() -> void:
	buffer._record_action("attack_light", 40)
	var result: bool = buffer.consume("attack_light", 8)
	assert_true(result, "Should consume a recently recorded action")

func test_consume_expired() -> void:
	buffer._record_action("attack_light", 40)
	# Advance past window
	for _i in range(10):
		buffer._physics_process(1.0 / 60.0)
	var result: bool = buffer.consume("attack_light", 8)
	assert_false(result, "Should not consume an expired input")

func test_consume_marks_consumed() -> void:
	buffer._record_action("attack_light", 40)
	var first: bool = buffer.consume("attack_light", 8)
	var second: bool = buffer.consume("attack_light", 8)
	assert_true(first, "First consume should succeed")
	assert_false(second, "Second consume of same input should fail")

func test_same_type_overwrite() -> void:
	buffer._record_action("attack_light", 40)
	# Record same action again — should overwrite
	buffer._record_action("attack_light", 40)
	assert_eq(buffer.get_active_count(), 1, "Same-type input should overwrite, keeping count at 1")

func test_consume_highest_priority() -> void:
	buffer._record_action("attack_light", 40)
	buffer._record_action("attack_heavy", 50)
	var result: String = buffer.consume_highest_priority(8)
	assert_eq(result, "attack_heavy", "Should return highest priority action")

func test_consume_highest_priority_empty() -> void:
	var result: String = buffer.consume_highest_priority(8)
	assert_eq(result, "", "Should return empty string when no inputs buffered")

func test_clear_empties_buffer() -> void:
	buffer._record_action("attack_light", 40)
	buffer.clear()
	assert_eq(buffer.get_active_count(), 0, "Buffer should be empty after clear")

func test_max_buffer_size() -> void:
	# Record more than MAX_BUFFER_SIZE inputs
	var actions: Array[String] = ["dodge", "attack_special", "attack_heavy", "attack_light", "jump"]
	for _round in range(2):
		for action: String in actions:
			buffer._record_action(action, InputBuffer.PRIORITY_MAP.get(action, 30))
	# After overflow, oldest should be pruned
	assert_lte(buffer._buffer.size(), InputBuffer.MAX_BUFFER_SIZE, "Buffer should not exceed MAX_BUFFER_SIZE")

func test_prune_consumed() -> void:
	buffer._record_action("attack_light", 40)
	buffer.consume("attack_light", 8)
	# Advance frames to trigger pruning
	for _i in range(20):
		buffer._physics_process(1.0 / 60.0)
	# The consumed input should have been pruned
	assert_eq(buffer._buffer.size(), 0, "Consumed inputs should be pruned")

func test_is_action_pressed_delegates() -> void:
	# is_action_pressed delegates to Input.is_action_pressed
	# Without actual input, should return false
	assert_false(buffer.is_action_pressed("move_left"), "Should return false when no key pressed")

func test_get_vector_delegates() -> void:
	# get_vector delegates to Input.get_axis
	# Without actual input, should return 0.0
	var result: float = buffer.get_vector("move_left", "move_right")
	assert_eq(result, 0.0, "Should return 0.0 when no keys pressed")

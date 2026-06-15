extends "res://addons/gut/test.gd"

var timer: FrameTimer

func before_each() -> void:
	timer = FrameTimer.new()

func after_each() -> void:
	timer = null

func test_start_sets_remaining() -> void:
	timer.start(10)
	assert_eq(timer.get_remaining(), 10, "Remaining should be 10 after start(10)")
	assert_true(timer.is_running(), "Timer should be running after start")

func test_tick_decrements() -> void:
	timer.start(5)
	timer.tick()
	timer.tick()
	timer.tick()
	assert_eq(timer.get_remaining(), 2, "Remaining should be 2 after 3 ticks on start(5)")

func test_expired_signal() -> void:
	var signal_count: int = 0
	timer.expired.connect(func(): signal_count += 1)
	timer.start(2)
	timer.tick()
	assert_eq(signal_count, 0, "Should not expire after 1 tick")
	timer.tick()
	assert_eq(signal_count, 1, "Should expire after 2 ticks")

func test_expired_not_running() -> void:
	timer.start(2)
	timer.tick()
	timer.tick()
	assert_false(timer.is_running(), "Should not be running after expiry")

func test_start_zero() -> void:
	timer.start(0)
	assert_false(timer.is_running(), "Should not be running after start(0)")
	assert_eq(timer.get_remaining(), 0, "Remaining should be 0")

func test_start_negative() -> void:
	timer.start(-1)
	assert_false(timer.is_running(), "Should not be running after start(-1)")
	assert_eq(timer.get_remaining(), 0, "Remaining should be 0 for negative start")

func test_restart_resets() -> void:
	timer.start(10)
	timer.tick()
	timer.tick()
	timer.tick()
	timer.tick()
	timer.tick()
	timer.start(3)
	assert_eq(timer.get_remaining(), 3, "Remaining should reset to 3 after restart")
	assert_true(timer.is_running(), "Should be running after restart")

func test_stop() -> void:
	timer.start(10)
	timer.stop()
	assert_false(timer.is_running(), "Should not be running after stop")
	assert_eq(timer.get_remaining(), 0, "Remaining should be 0 after stop")

func test_get_elapsed() -> void:
	timer.start(10)
	for _i in range(4):
		timer.tick()
	assert_eq(timer.get_elapsed(), 4, "Elapsed should be 4 after 4 ticks")

func test_get_progress() -> void:
	timer.start(10)
	for _i in range(5):
		timer.tick()
	assert_eq(timer.get_progress(), 0.5, "Progress should be 0.5 at halfway")

func test_tick_when_not_running() -> void:
	# Should not crash when ticking without starting
	timer.tick()
	assert_eq(timer.get_remaining(), 0, "Should remain 0 when ticking without start")

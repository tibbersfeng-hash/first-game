extends "res://addons/gut/test.gd"

## Test states for StateMachine tests
class TestStateIdle extends State:
	var enter_called: bool = false
	var exit_called: bool = false
	var physics_update_count: int = 0

	func enter() -> void:
		enter_called = true

	func exit() -> void:
		exit_called = true

	func physics_update(_delta: float) -> void:
		physics_update_count += 1

class TestStateRun extends State:
	var enter_called: bool = false
	var exit_called: bool = false

	func enter() -> void:
		enter_called = true

	func exit() -> void:
		exit_called = true

var sm: StateMachine
var idle_state: TestStateIdle
var run_state: TestStateRun

func before_each() -> void:
	sm = StateMachine.new()
	idle_state = TestStateIdle.new()
	idle_state.name = "Idle"
	run_state = TestStateRun.new()
	run_state.name = "Run"
	sm.add_child(idle_state)
	sm.add_child(run_state)
	add_child(sm)
	# _ready will _collect_states and set initial if specified

func after_each() -> void:
	sm.queue_free()

func test_initial_state_set() -> void:
	sm.initial_state_name = "Idle"
	sm._ready()
	assert_not_null(sm.current_state, "Current state should be set")
	assert_eq(sm.get_current_state_name(), "Idle", "Current state should be Idle")

func test_transition_changes_state() -> void:
	sm.initial_state_name = "Idle"
	sm._ready()
	sm.transition_to("Run")
	assert_eq(sm.get_current_state_name(), "Run", "Should transition to Run state")

func test_frame_in_state_increments() -> void:
	sm.initial_state_name = "Idle"
	sm._ready()
	for _i in range(3):
		sm._physics_process(1.0 / 60.0)
	assert_eq(sm.frame_in_state, 3, "frame_in_state should be 3 after 3 physics ticks")

func test_frame_in_state_resets_on_transition() -> void:
	sm.initial_state_name = "Idle"
	sm._ready()
	for _i in range(5):
		sm._physics_process(1.0 / 60.0)
	sm.transition_to("Run")
	assert_eq(sm.frame_in_state, 0, "frame_in_state should reset to 0 on transition")

func test_enter_exit_called() -> void:
	sm.initial_state_name = "Idle"
	sm._ready()
	# Reset flags set by initial transition
	idle_state.enter_called = false
	idle_state.exit_called = false
	run_state.enter_called = false

	sm.transition_to("Run")
	assert_true(idle_state.exit_called, "Old state exit() should be called")
	assert_true(run_state.enter_called, "New state enter() should be called")

func test_state_entered_signal() -> void:
	var signal_name: String = ""
	sm.state_entered.connect(func(n): signal_name = n)
	sm.initial_state_name = "Idle"
	sm._ready()
	assert_eq(signal_name, "Idle", "state_entered should fire with state name")

func test_state_exited_signal() -> void:
	var signal_name: String = ""
	sm.state_exited.connect(func(n): signal_name = n)
	sm.initial_state_name = "Idle"
	sm._ready()
	sm.transition_to("Run")
	assert_eq(signal_name, "Idle", "state_exited should fire with old state name")

func test_transition_to_self_noop() -> void:
	sm.initial_state_name = "Idle"
	sm._ready()
	idle_state.enter_called = false
	sm.transition_to("Idle")
	assert_false(idle_state.enter_called, "Re-entering same state should be no-op")

func test_unknown_state_warning() -> void:
	sm.initial_state_name = "Idle"
	sm._ready()
	# Should not crash
	sm.transition_to("Nonexistent")
	assert_eq(sm.get_current_state_name(), "Idle", "Should stay in current state on unknown transition")

func test_get_state() -> void:
	sm._ready()
	var state: State = sm.get_state("idle")
	assert_not_null(state, "Should find state by lowercase name")
	assert_eq(state.name, "Idle", "Should return correct state")

func test_get_state_not_found() -> void:
	sm._ready()
	var state: State = sm.get_state("nonexistent")
	assert_null(state, "Should return null for unknown state")

## Damage number pool — object pool for floating damage text.
## ADR-003: Listens to SignalBus.damage_number_requested.
## Avoids frequent instantiate/queue_free that causes GC pressure.
extends Node

const POOL_SIZE: int = 10

var _pool: Array[Label] = []
var _scene: PackedScene = preload("res://src/presentation/effects/damage_number.tscn")

func _ready() -> void:
	SignalBus.damage_number_requested.connect(_on_damage_number_requested)
	for _i: int in POOL_SIZE:
		var node: Label = _scene.instantiate()
		node.visible = false
		node.set_physics_process(false)
		add_child(node)
		_pool.append(node)

func _on_damage_number_requested(pos: Vector2, damage: int, damage_type: int, is_crit: bool) -> void:
	var node: Label = _get_available()
	if node:
		node.spawn(pos, damage, damage_type, is_crit)

func _get_available() -> Label:
	for node: Label in _pool:
		if not node.visible:
			return node
	# Pool exhausted — expand
	var new_node: Label = _scene.instantiate()
	new_node.visible = false
	new_node.set_physics_process(false)
	add_child(new_node)
	_pool.append(new_node)
	return new_node

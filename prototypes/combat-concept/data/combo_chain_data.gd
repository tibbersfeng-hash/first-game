# Combo chain data — defines a sequence of attacks
class_name ComboChainData
extends Resource

@export var chain_name: String = ""
@export var display_name: String = ""
@export var steps: Array[AttackStepData] = []
@export var timeout_frames: int = 48         # 0.8s at 60fps, used when step window_frames = 0

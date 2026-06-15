## Combo chain definition — a sequence of AttackData steps.
## ADR-004: all values have type annotations and default values.
class_name ComboData
extends Resource

@export var combo_id: String = ""
@export var display_name: String = ""
@export var chain_name: String = ""
@export var steps: Array[AttackData] = []
@export var timeout_frames: int = 48  # 0.8s at 60fps, used when step window_frames = 0
@export var cancel_window_pct: float = 0.5  # recovery帧的前50%可取消
@export var bonus_damage_pct: float = 0.0

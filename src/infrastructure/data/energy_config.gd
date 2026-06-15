## Energy system configuration.
## Per combat-system GDD: energy gains on hit, spent on special/heavy/dodge.
class_name EnergyConfig
extends Resource

@export var max_energy: int = 100
@export var starting_energy: int = 0
@export var special_threshold: int = 30
@export var dodge_cost: int = 8
@export var heavy_cost: int = 5
@export var hurt_gain: int = 3

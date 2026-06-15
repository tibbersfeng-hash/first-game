## Reusable energy management component.
## Per combat-system GDD: energy gains on hit, spent on special/heavy/dodge.
## Per ADR-004: all values data-driven from EnergyConfig Resource.
class_name EnergyComponent
extends Node

@export var max_energy: int = 100
@export var starting_energy: int = 0
@export var special_threshold: int = 30
@export var dodge_cost: int = 8
@export var heavy_cost: int = 5

var current_energy: int = 0

signal energy_changed(current: int, maximum: int)

func _ready() -> void:
	current_energy = starting_energy

## Gain energy (from hitting enemies or being hit). Clamped to max.
func gain(amount: int) -> void:
	if amount <= 0:
		return
	current_energy = mini(current_energy + amount, max_energy)
	energy_changed.emit(current_energy, max_energy)

## Spend energy if sufficient. Returns true if successful.
func spend(amount: int) -> bool:
	if not can_spend(amount):
		return false
	current_energy -= amount
	energy_changed.emit(current_energy, max_energy)
	return true

## Check if the entity can afford to spend the given amount.
func can_spend(amount: int) -> bool:
	return current_energy >= amount

## Check if special attack is available (energy >= threshold).
func can_special() -> bool:
	return current_energy >= special_threshold

## Check if dodge is available.
func can_dodge() -> bool:
	return current_energy >= dodge_cost

## Check if heavy attack is available.
func can_heavy() -> bool:
	return current_energy >= heavy_cost

## Get energy as a 0.0-1.0 ratio.
func get_energy_ratio() -> float:
	if max_energy == 0:
		return 0.0
	return float(current_energy) / float(max_energy)

## Reset energy to starting value.
func reset() -> void:
	current_energy = starting_energy
	energy_changed.emit(current_energy, max_energy)

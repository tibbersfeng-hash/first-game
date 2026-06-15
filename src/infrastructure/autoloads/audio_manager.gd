## Audio playback autoload (ADR-002: command-only).
## Accepts play requests, never calls back into business logic.
## Uses a pool of AudioStreamPlayer nodes for concurrent SFX playback.
extends Node

var _sfx_players: Array[AudioStreamPlayer] = []
var _bgm_player: AudioStreamPlayer = null
const MAX_SFX_PLAYERS: int = 8

func _ready() -> void:
	_bgm_player = AudioStreamPlayer.new()
	add_child(_bgm_player)
	for _i: int in MAX_SFX_PLAYERS:
		var player: AudioStreamPlayer = AudioStreamPlayer.new()
		add_child(player)
		_sfx_players.append(player)

## Play a one-shot sound effect. Uses the first available player.
func play_sfx(stream: AudioStream) -> void:
	for player: AudioStreamPlayer in _sfx_players:
		if not player.playing:
			player.stream = stream
			player.play()
			return
	# All players busy — steal the first one
	_sfx_players[0].stream = stream
	_sfx_players[0].play()

## Play background music with optional fade-in.
func play_bgm(stream: AudioStream, _fade_time: float = 1.0) -> void:
	_bgm_player.stream = stream
	_bgm_player.play()

## Stop background music with optional fade-out.
func stop_bgm(_fade_time: float = 1.0) -> void:
	_bgm_player.stop()

## Stub for combat hit SFX — will be expanded with audio resources.
func play_combat_hit_sfx(_damage_type: int) -> void:
	pass

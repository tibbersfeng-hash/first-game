extends Node

var _frame := 0
var _scene: Node = null

func _ready() -> void:
	_scene = load("res://src/gameplay/scenes/combat_test.tscn").instantiate()
	add_child(_scene)
	var player = _scene.get_node("Player")
	player.position = Vector2(150, 200)
	player.facing = 1
	Engine.max_fps = 60
	print("Ready renderer=%s" % RenderingServer.get_video_adapter_name())

func _process(delta: float) -> void:
	_frame += 1
	if _frame == 120:
		# 强制渲染一帧然后截图
		RenderingServer.force_draw()
		await RenderingServer.frame_post_draw
		var img = get_viewport().get_texture().get_image()
		if img:
			# 检查多个位置
			for pos in [Vector2i(640,360), Vector2i(250,360), Vector2i(600,400), Vector2i(100,100), Vector2i(1100,600)]:
				var px = img.get_pixel(pos.x, pos.y)
				print("  px(%d,%d)=%s" % [pos.x, pos.y, str(px)])
			img.save_png("/tmp/final_ss.png")
			print("Saved!")
		get_tree().quit()

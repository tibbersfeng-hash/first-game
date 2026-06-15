extends Node

var _viewport: SubViewport = null
var _frame := 0

func _ready() -> void:
	# 创建 SubViewport
	_viewport = SubViewport.new()
	_viewport.size = Vector2i(1280, 720)
	_viewport.render_target_update_mode = SubViewport.UPDATE_ALWAYS
	_viewport.render_target_clear_mode = SubViewport.CLEAR_MODE_ALWAYS
	add_child(_viewport)
	
	# 把战斗场景加到 SubViewport 里
	var scene = load("res://src/gameplay/scenes/combat_test.tscn").instantiate()
	_viewport.add_child(scene)
	
	var player = scene.get_node("Player")
	player.position = Vector2(150, 200)
	player.facing = 1
	
	print("Setup done")

func _process(delta: float) -> void:
	_frame += 1
	if _frame == 60:
		# 从 SubViewport 截图
		var img = _viewport.get_texture().get_image()
		if img:
			img.save_png("/tmp/sv_test.png")
			# 分析
			var px = img.get_pixel(640, 360)
			print("Center pixel: %s" % str(px))
			print("Image size: %s" % str(img.get_size()))
			# 也试 viewport 0
			var main_img = get_viewport().get_texture().get_image()
			if main_img:
				main_img.save_png("/tmp/sv_main.png")
				var px2 = main_img.get_pixel(640, 360)
				print("Main center: %s" % str(px2))
		get_tree().quit()

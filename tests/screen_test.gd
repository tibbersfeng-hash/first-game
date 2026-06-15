extends Node

var _frame := 0

func _ready() -> void:
	var scene = load("res://src/gameplay/scenes/combat_test.tscn").instantiate()
	add_child(scene)
	var player = scene.get_node("Player")
	player.position = Vector2(150, 200)
	player.facing = 1
	print("Ready")

func _process(delta: float) -> void:
	_frame += 1
	if _frame == 60:
		print("Taking screenshot...")
		# 方法1: get_viewport
		var vp = get_viewport()
		var tex = vp.get_texture()
		print("  viewport size: %s" % str(vp.get_visible_rect()))
		print("  texture: %s" % str(tex))
		print("  texture size: %s" % str(tex.get_size() if tex else "null"))
		if tex:
			var img = tex.get_image()
			if img:
				print("  image size: %s" % str(img.get_size()))
				img.save_png("/tmp/vp_screenshot.png")
				print("  saved! center_px=%s" % str(img.get_pixel(640, 360)))
			else:
				print("  image is null!")
		get_tree().quit()

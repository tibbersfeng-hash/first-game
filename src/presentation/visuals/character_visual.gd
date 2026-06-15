extends Node2D
class_name CharacterVisual

## Q版中国武侠风角色绘制
## 原创设计，灵感来源于中国传统武侠/仙侠美术风格

@export var body_color: Color = Color(0.3, 0.5, 0.9)  # 蓝色武服
@export var skin_color: Color = Color(1.0, 0.85, 0.75)
@export var outline_color: Color = Color(0.1, 0.1, 0.15)
@export var is_enemy: bool = false
@export var scale_factor: float = 4.0
@export var facing_right: bool = true

var _anim_frame := 0
var _anim_timer := 0.0

const HEAD_R = 28.0
const BODY_W = 35.0
const BODY_H = 40.0
const LEG_W = 12.0
const LEG_H = 25.0
const ARM_W = 10.0
const ARM_H = 30.0

func _ready() -> void:
	set_process(true)

func _process(delta: float) -> void:
	_anim_timer += delta
	if _anim_timer > 0.3:
		_anim_timer = 0
		_anim_frame = (_anim_frame + 1) % 4
		queue_redraw()

func _draw() -> void:
	var s = scale_factor
	var flip = 1.0 if facing_right else -1.0

	if is_enemy:
		_draw_enemy(s, flip)
	else:
		_draw_player(s, flip)

func _draw_player(s: float, flip: float) -> void:
	# === 玩家：蓝色武僧 ===
	var body_y = -BODY_H * s

	# 腿 (带白色绑腿)
	for side in [-1, 1]:
		var leg_x = side * 10 * s * flip
		# 裤子
		draw_rect(Rect2(leg_x - LEG_W*s/2, body_y + BODY_H*s, LEG_W*s, LEG_H*s), body_color)
		draw_rect(Rect2(leg_x - LEG_W*s/2, body_y + BODY_H*s, LEG_W*s, LEG_H*s), outline_color, false, 2.0)
		# 白色绑腿
		for i in range(3):
			var wrap_y = body_y + BODY_H*s + LEG_H*s - (i+1)*6*s
			draw_rect(Rect2(leg_x - LEG_W*s/2, wrap_y, LEG_W*s, 4*s), Color(0.95, 0.95, 0.9))

	# 身体 (武服)
	draw_rect(Rect2(-BODY_W*s/2, body_y, BODY_W*s, BODY_H*s), body_color)
	draw_rect(Rect2(-BODY_W*s/2, body_y, BODY_W*s, BODY_H*s), outline_color, false, 3.0)

	# 红色腰带
	draw_rect(Rect2(-BODY_W*s/2, body_y + BODY_H*s*0.6, BODY_W*s, 8*s), Color(0.9, 0.2, 0.2))
	draw_rect(Rect2(-BODY_W*s/2, body_y + BODY_H*s*0.6, BODY_W*s, 8*s), outline_color, false, 2.0)
	# 腰带结
	draw_circle(Vector2(0, body_y + BODY_H*s*0.6 + 4*s), 5*s, Color(0.9, 0.2, 0.2))

	# 衣领 (V领)
	draw_line(Vector2(-8*s, body_y), Vector2(0, body_y + 15*s), Color(0.95, 0.95, 0.9), 3*s)
	draw_line(Vector2(8*s, body_y), Vector2(0, body_y + 15*s), Color(0.95, 0.95, 0.9), 3*s)

	# 手臂 (带护腕)
	for side in [-1, 1]:
		var arm_x = side * (BODY_W*s/2 + ARM_W*s/2)
		# 袖子
		draw_rect(Rect2(arm_x - ARM_W*s/2, body_y + 5*s, ARM_W*s, ARM_H*s*0.6), body_color)
		draw_rect(Rect2(arm_x - ARM_W*s/2, body_y + 5*s, ARM_W*s, ARM_H*s*0.6), outline_color, false, 2.0)
		# 前臂 (肤色)
		draw_rect(Rect2(arm_x - ARM_W*s/2, body_y + 5*s + ARM_H*s*0.6, ARM_W*s, ARM_H*s*0.4), skin_color)
		# 护腕
		draw_rect(Rect2(arm_x - ARM_W*s/2 - 1*s, body_y + 5*s + ARM_H*s*0.5, ARM_W*s + 2*s, 5*s), Color(0.3, 0.3, 0.3))

	# 拳头特效 (金色光芒)
	var punch_offset = sin(_anim_frame * PI/2) * 3 * s
	for side in [-1, 1]:
		var fist_x = side * (BODY_W*s/2 + ARM_W*s/2)
		var fist_y = body_y + 5*s + ARM_H*s + punch_offset * side
		# 拳头
		draw_circle(Vector2(fist_x, fist_y), 6*s, skin_color)
		draw_circle(Vector2(fist_x, fist_y), 6*s, outline_color, false, 2.0)
		# 金色光芒
		if _anim_frame % 2 == 0:
			draw_circle(Vector2(fist_x, fist_y), 10*s, Color(1.0, 0.8, 0.2, 0.3))

	# 头部
	var head_y = body_y - HEAD_R * s
	draw_circle(Vector2(0, head_y), HEAD_R*s, skin_color)
	draw_circle(Vector2(0, head_y), HEAD_R*s, outline_color, false, 3.0)

	# 头带 (红色)
	draw_arc(Vector2(0, head_y), HEAD_R*s, PI*0.8, PI*0.2, 20, Color(0.9, 0.2, 0.2), 6*s)
	# 头带飘带
	draw_line(Vector2(HEAD_R*s*0.7*flip, head_y - HEAD_R*s*0.3),
			  Vector2(HEAD_R*s*1.2*flip, head_y - HEAD_R*s*0.5),
			  Color(0.9, 0.2, 0.2), 4*s)

	# 头发 (黑色)
	draw_circle(Vector2(-8*s*flip, head_y - HEAD_R*s*0.3), 10*s, Color(0.15, 0.15, 0.2))
	draw_circle(Vector2(8*s*flip, head_y - HEAD_R*s*0.3), 10*s, Color(0.15, 0.15, 0.2))
	draw_circle(Vector2(0, head_y - HEAD_R*s*0.5), 12*s, Color(0.15, 0.15, 0.2))

	# 眼睛 (大而有神)
	for side in [-1, 1]:
		var eye_x = side * 10 * s * flip
		var eye_y = head_y + 2*s
		# 眼白
		draw_circle(Vector2(eye_x, eye_y), 6*s, Color.WHITE)
		# 瞳孔
		draw_circle(Vector2(eye_x + 2*s*flip, eye_y), 3.5*s, Color(0.1, 0.1, 0.2))
		# 高光
		draw_circle(Vector2(eye_x + 3*s*flip, eye_y - 1*s), 1.5*s, Color.WHITE)
		# 眉毛 (粗黑)
		draw_line(Vector2(eye_x - 6*s, eye_y - 8*s),
				  Vector2(eye_x + 6*s, eye_y - 9*s),
				  Color(0.1, 0.1, 0.2), 3*s)

	# 嘴巴 (坚毅)
	draw_arc(Vector2(0, head_y + 12*s), 6*s, 0.2, PI - 0.2, 10, outline_color, 2.5)

func _draw_enemy(s: float, flip: float) -> void:
	# === 敌人：绿色小妖怪 ===
	var body_y = -BODY_H * s

	# 腿 (棕色短裤)
	for side in [-1, 1]:
		var leg_x = side * 10 * s * flip
		# 短裤
		draw_rect(Rect2(leg_x - LEG_W*s/2, body_y + BODY_H*s, LEG_W*s, LEG_H*s*0.5), Color(0.5, 0.35, 0.2))
		draw_rect(Rect2(leg_x - LEG_W*s/2, body_y + BODY_H*s, LEG_W*s, LEG_H*s*0.5), outline_color, false, 2.0)
		# 绿色小腿
		draw_rect(Rect2(leg_x - LEG_W*s/2, body_y + BODY_H*s + LEG_H*s*0.5, LEG_W*s, LEG_H*s*0.5), Color(0.4, 0.7, 0.3))
		draw_rect(Rect2(leg_x - LEG_W*s/2, body_y + BODY_H*s + LEG_H*s*0.5, LEG_W*s, LEG_H*s*0.5), outline_color, false, 2.0)

	# 身体 (红色肚兜)
	draw_rect(Rect2(-BODY_W*s/2, body_y, BODY_W*s, BODY_H*s), Color(0.85, 0.2, 0.2))
	draw_rect(Rect2(-BODY_W*s/2, body_y, BODY_W*s, BODY_H*s), outline_color, false, 3.0)
	# 肚兜带子
	draw_line(Vector2(-BODY_W*s/2, body_y + 5*s), Vector2(BODY_W*s/2, body_y + 5*s), Color(0.6, 0.15, 0.15), 3*s)
	draw_line(Vector2(0, body_y), Vector2(0, body_y + BODY_H*s), Color(0.6, 0.15, 0.15), 3*s)

	# 手臂 (绿色)
	for side in [-1, 1]:
		var arm_x = side * (BODY_W*s/2 + ARM_W*s/2)
		draw_rect(Rect2(arm_x - ARM_W*s/2, body_y + 5*s, ARM_W*s, ARM_H*s), Color(0.4, 0.7, 0.3))
		draw_rect(Rect2(arm_x - ARM_W*s/2, body_y + 5*s, ARM_W*s, ARM_H*s), outline_color, false, 2.0)

	# 武器 (木棍)
	var staff_x = (BODY_W*s/2 + ARM_W*s) * flip
	draw_rect(Rect2(staff_x - 3*s, body_y - 10*s, 6*s, 60*s), Color(0.5, 0.35, 0.2))
	draw_rect(Rect2(staff_x - 3*s, body_y - 10*s, 6*s, 60*s), outline_color, false, 2.0)
	# 棍子顶端
	draw_circle(Vector2(staff_x, body_y - 10*s), 5*s, Color(0.6, 0.4, 0.25))

	# 头部 (绿色)
	var head_y = body_y - HEAD_R * s
	draw_circle(Vector2(0, head_y), HEAD_R*s, Color(0.4, 0.7, 0.3))
	draw_circle(Vector2(0, head_y), HEAD_R*s, outline_color, false, 3.0)

	# 尖耳朵
	for side in [-1, 1]:
		var ear_x = side * HEAD_R * s * 0.9
		var ear_points = PackedVector2Array([
			Vector2(ear_x, head_y),
			Vector2(ear_x + side * 12*s, head_y - 8*s),
			Vector2(ear_x + side * 5*s, head_y + 5*s)
		])
		draw_colored_polygon(ear_points, Color(0.4, 0.7, 0.3))
		draw_polyline(ear_points, outline_color, 2.0)

	# 尖角 (头顶两个角)
	for i in range(2):
		var horn_x = (i * 2 - 1) * 10 * s
		var horn_points = PackedVector2Array([
			Vector2(horn_x - 5*s, head_y - HEAD_R*s*0.7),
			Vector2(horn_x, head_y - HEAD_R*s*1.3),
			Vector2(horn_x + 5*s, head_y - HEAD_R*s*0.7)
		])
		draw_colored_polygon(horn_points, Color(0.6, 0.2, 0.2))
		draw_polyline(horn_points, outline_color, 2.0)

	# 眼睛 (凶狠)
	for side in [-1, 1]:
		var eye_x = side * 10 * s * flip
		var eye_y = head_y
		# 眼白 (较小)
		draw_circle(Vector2(eye_x, eye_y), 5*s, Color(1.0, 1.0, 0.8))
		# 红色瞳孔
		draw_circle(Vector2(eye_x + 1*s*flip, eye_y), 3*s, Color(0.8, 0.1, 0.1))
		# 凶狠眉毛
		draw_line(Vector2(eye_x - 7*s, eye_y - 7*s),
				  Vector2(eye_x + 5*s*flip, eye_y - 10*s),
				  Color(0.1, 0.1, 0.2), 3.5*s)

	# 嘴巴 (锯齿状，凶狠)
	var mouth_y = head_y + 12*s
	draw_line(Vector2(-8*s, mouth_y), Vector2(-4*s, mouth_y + 4*s), outline_color, 2.5)
	draw_line(Vector2(-4*s, mouth_y + 4*s), Vector2(0, mouth_y), outline_color, 2.5)
	draw_line(Vector2(0, mouth_y), Vector2(4*s, mouth_y + 4*s), outline_color, 2.5)
	draw_line(Vector2(4*s, mouth_y + 4*s), Vector2(8*s, mouth_y), outline_color, 2.5)

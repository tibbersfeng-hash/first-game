extends Node2D
class_name CharacterVisual

## Q版像素风角色绘制 — 用 _draw() 替代 ColorRect 占位符
## 参数通过 @export 控制外观

@export var body_color: Color = Color(0.4, 0.6, 1.0)
@export var head_color: Color = Color(1.0, 0.85, 0.7)  # 肤色
@export var eye_color: Color = Color.WHITE
@export var pupil_color: Color = Color(0.1, 0.1, 0.2)
@export var outline_color: Color = Color(0.15, 0.15, 0.2)
@export var is_enemy: bool = false
@export var scale_factor: float = 1.0

const HEAD_SIZE = 50.0
const BODY_W = 45.0
const BODY_H = 60.0
const LEG_W = 18.0
const LEG_H = 35.0
const ARM_W = 14.0
const ARM_H = 45.0

func _draw() -> void:
	var s = scale_factor
	var body_offset_y = -BODY_H * s

	# === 腿 (2条) ===
	# 左腿
	draw_rect(Rect2(-22*s, body_offset_y + BODY_H*s, LEG_W*s, LEG_H*s), body_color)
	draw_rect(Rect2(-22*s, body_offset_y + BODY_H*s, LEG_W*s, LEG_H*s), outline_color, false, 2.0)
	# 右腿
	draw_rect(Rect2(4*s, body_offset_y + BODY_H*s, LEG_W*s, LEG_H*s), body_color)
	draw_rect(Rect2(4*s, body_offset_y + BODY_H*s, LEG_W*s, LEG_H*s), outline_color, false, 2.0)

	# === 身体 (躯干) ===
	draw_rect(Rect2(-BODY_W*s/2, body_offset_y, BODY_W*s, BODY_H*s), body_color)
	draw_rect(Rect2(-BODY_W*s/2, body_offset_y, BODY_W*s, BODY_H*s), outline_color, false, 3.0)

	# === 手臂 (2条, 左右) ===
	# 左臂
	draw_rect(Rect2(-BODY_W*s/2 - ARM_W*s, body_offset_y + 5*s, ARM_W*s, ARM_H*s), body_color)
	draw_rect(Rect2(-BODY_W*s/2 - ARM_W*s, body_offset_y + 5*s, ARM_W*s, ARM_H*s), outline_color, false, 2.0)
	# 右臂
	draw_rect(Rect2(BODY_W*s/2, body_offset_y + 5*s, ARM_W*s, ARM_H*s), body_color)
	draw_rect(Rect2(BODY_W*s/2, body_offset_y + 5*s, ARM_W*s, ARM_H*s), outline_color, false, 2.0)

	# === 武器 (右手持) — 敌人是棍棒, 玩家是拳套 ===
	if is_enemy:
		# 哥布林棍棒
		draw_rect(Rect2(BODY_W*s/2 + ARM_W*s - 4*s, body_offset_y + 10*s, 12*s, 55*s), Color(0.5, 0.35, 0.2))
		draw_rect(Rect2(BODY_W*s/2 + ARM_W*s - 8*s, body_offset_y + 5*s, 20*s, 15*s), Color(0.6, 0.4, 0.2))
	else:
		# 拳套
		draw_circle(Vector2(BODY_W*s/2 + ARM_W*s + 5*s, body_offset_y + ARM_H*s + 5*s), 12*s, Color(0.8, 0.2, 0.2))

	# === 头部 (大圆, Q版大头) ===
	var head_center = Vector2(0, body_offset_y - HEAD_SIZE*s*0.5)
	draw_circle(head_center, HEAD_SIZE*s/2, head_color)
	draw_circle(head_center, HEAD_SIZE*s/2, outline_color, false, 3.0)

	# === 头发/角 ===
	if is_enemy:
		# 哥布林尖角
		draw_colored_polygon(PackedVector2Array([
			Vector2(-8*s, body_offset_y - HEAD_SIZE*s*0.4),
			Vector2(0, body_offset_y - HEAD_SIZE*s*0.8),
			Vector2(8*s, body_offset_y - HEAD_SIZE*s*0.4)
		]), Color(0.5, 0.2, 0.2))
		draw_colored_polygon(PackedVector2Array([
			Vector2(6*s, body_offset_y - HEAD_SIZE*s*0.35),
			Vector2(12*s, body_offset_y - HEAD_SIZE*s*0.7),
			Vector2(18*s, body_offset_y - HEAD_SIZE*s*0.3)
		]), Color(0.5, 0.2, 0.2))
	else:
		# 玩家刘海
		draw_circle(Vector2(-10*s, body_offset_y - HEAD_SIZE*s*0.55), 12*s, Color(0.3, 0.2, 0.15))
		draw_circle(Vector2(10*s, body_offset_y - HEAD_SIZE*s*0.55), 12*s, Color(0.3, 0.2, 0.15))

	# === 眼睛 ===
	var eye_y = body_offset_y - HEAD_SIZE*s*0.5
	# 眼白
	draw_circle(Vector2(-9*s, eye_y), 7*s, eye_color)
	draw_circle(Vector2(9*s, eye_y), 7*s, eye_color)
	# 瞳孔
	draw_circle(Vector2(-7*s, eye_y), 3.5*s, pupil_color)
	draw_circle(Vector2(11*s, eye_y), 3.5*s, pupil_color)

	# === 嘴巴 ===
	if is_enemy:
		# 敌人锯齿嘴
		draw_line(Vector2(-8*s, eye_y + 14*s), Vector2(0, eye_y + 18*s), outline_color, 2.5)
		draw_line(Vector2(0, eye_y + 18*s), Vector2(8*s, eye_y + 14*s), outline_color, 2.5)
	else:
		# 玩家微笑
		draw_arc(Vector2(0, eye_y + 12*s), 8*s, 0.2, PI - 0.2, 12, outline_color, 2.5)

	# === 敌人额外特征: 耳朵 ===
	if is_enemy:
		draw_circle(Vector2(-HEAD_SIZE*s/2, eye_y), 8*s, head_color)
		draw_circle(Vector2(-HEAD_SIZE*s/2, eye_y), 8*s, outline_color, false, 2.0)
		draw_circle(Vector2(HEAD_SIZE*s/2, eye_y), 8*s, head_color)
		draw_circle(Vector2(HEAD_SIZE*s/2, eye_y), 8*s, outline_color, false, 2.0)

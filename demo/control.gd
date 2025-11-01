extends Control

var lights = GDGameLights.new()
var pulsing: bool = false
var tween: Tween = null
var blue_intensity: float = 0.0

@onready var btn_2 : BaseButton = $Button2

func _ready() -> void:
	lights.connect_to_openrgb("127.0.0.1", 6742)
	lights.set_all_devices_color(Color(1, 1, 0))

func _on_button_pressed() -> void:
	pulsing = false
	btn_2.text = "Animate it Pulsing Blue!"
	if tween:
		tween.kill()
		tween = null
	lights.set_all_devices_color(Color(1, 0, 0))  # immediate red

func _on_button_2_pressed() -> void:
	pulsing = not pulsing
	if pulsing:
		btn_2.text = "Stop Pulsing Blue!"
		start_pulse()
	else:
		btn_2.text = "Animate it Pulsing Blue!"
		if tween:
			tween.kill()
			tween = null
		blue_intensity = 0.0
		update_blue_intensity()

func update_blue_intensity() -> void:
	lights.set_all_devices_color(Color(0, 0, blue_intensity))

func start_pulse() -> void:
	if tween:
		tween.kill()
	
	tween = create_tween()
	var duration = 1.0
	
	# Animate blue_intensity from 0 -> 1
	tween.tween_method(Callable(self, "set_blue_intensity"), 0.0, 1.0, duration / 2)
	tween.set_trans(Tween.TRANS_SINE).set_ease(Tween.EASE_IN_OUT)
	# Animate blue_intensity from 1 -> 0
	tween.tween_method(Callable(self, "set_blue_intensity"), 1.0, 0.0, duration / 2)
	tween.set_trans(Tween.TRANS_SINE).set_ease(Tween.EASE_IN_OUT)

	tween.set_loops()
	tween.play()

func set_blue_intensity(value: float) -> void:
	blue_intensity = value
	update_blue_intensity()

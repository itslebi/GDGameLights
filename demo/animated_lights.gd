extends Control


var lights = GDAnimatedGameLights.new()
@onready var colorPicker: ColorPicker = $ColorPicker
@onready var current_color: Color = colorPicker.color

func _ready() -> void:
	add_child(lights)
	lights.connect_to_openrgb()
	lights.set_all_devices_to_direct_mode() 

func _on_color_changed(new_color: Color) -> void:
	current_color = new_color

func _on_static_pressed() -> void:
	lights.mode = GDAnimatedGameLights.MODE_STATIC
	lights.start_animation(2.0, current_color)

func _on_pulse_pressed() -> void:
	lights.mode = GDAnimatedGameLights.MODE_PULSE
	lights.start_animation(1.5, current_color)

func _on_rainbow_pressed() -> void:
	lights.mode = GDAnimatedGameLights.MODE_RAINBOW
	lights.start_animation(1.0, current_color)

func _on_stop_pressed() -> void:
	lights.end_animation()

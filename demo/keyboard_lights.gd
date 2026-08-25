extends Control

@onready var lights = $GDKeyboardGameLights

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	lights.connect_to_openrgb()
	lights.set_all_devices_to_direct_mode()

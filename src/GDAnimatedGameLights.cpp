#include <GDAnimatedGameLights.hpp>

using namespace godot;

GDAnimatedGameLights::GDAnimatedGameLights()
    : GDGameLights() {}

GDAnimatedGameLights::~GDAnimatedGameLights() {
    if (tween.is_valid() && tween->is_valid()) {
        tween->kill();
        tween.unref();
    }
}

// ----------- Bind Methods to be used inside GDScript
VARIANT_ENUM_CAST(GDAnimatedGameLights::AnimationMode);

void GDAnimatedGameLights::_bind_methods() {
    //Bind enum
    BIND_ENUM_CONSTANT(MODE_OFF);
    BIND_ENUM_CONSTANT(MODE_PULSE);
    BIND_ENUM_CONSTANT(MODE_STATIC);
    BIND_ENUM_CONSTANT(MODE_RAINBOW);
    BIND_ENUM_CONSTANT(MODE_CUSTOM);

    ClassDB::bind_method(D_METHOD("set_mode", "p_mode"), 
                         &GDAnimatedGameLights::set_mode);
    ClassDB::bind_method(D_METHOD("get_mode"), 
                         &GDAnimatedGameLights::get_mode);

    ADD_PROPERTY(
        PropertyInfo(
            Variant::INT,
            "mode",
            PROPERTY_HINT_ENUM,
            "Off,Static,Pulse,Rainbow,Custom"
        ),
        "set_mode",
        "get_mode"
    );

    //Methods
    ClassDB::bind_method(D_METHOD("start_animation", "duration", "color"),
                         &GDAnimatedGameLights::start_animation,
                         DEFVAL(1.0),
                         DEFVAL(Color(0.0, 0.0, 1.0)));
    ClassDB::bind_method(D_METHOD("end_animation"),
                         &GDAnimatedGameLights::end_animation);

    ClassDB::bind_method(D_METHOD("set_color_intensity", "intensity", "target_color"), 
                         &GDAnimatedGameLights::set_color_intensity);
}

// ----------- Methods

void GDAnimatedGameLights::set_mode(AnimationMode p_mode) noexcept {
    mode = p_mode;
}

GDAnimatedGameLights::AnimationMode GDAnimatedGameLights::get_mode() const noexcept {
    return mode;
}

void GDAnimatedGameLights::set_color_intensity(float intensity, Color target_color) noexcept {
    Color color = intensity * target_color;
    set_all_devices_color(color);
}

bool GDAnimatedGameLights::set_mode_pulse(float duration, Color color) noexcept {
	if (!tween.is_valid() || !tween->is_valid()) {
        tween = create_tween();
    }

    Callable callable = Callable(this, "set_color_intensity").bind(color);

    // Animate blue_intensity from 0 → 1
    tween->tween_method(callable, 0.0, 1.0, duration / 2.0);
    tween->set_trans(Tween::TransitionType::TRANS_SINE);
    tween->set_ease(Tween::EaseType::EASE_IN_OUT);

    // Animate blue_intensity from 1 → 0
    tween->tween_method(callable, 1.0, 0.0, duration / 2.0);
    tween->set_trans(Tween::TransitionType::TRANS_SINE);
    tween->set_ease(Tween::EaseType::EASE_IN_OUT);

    tween->set_loops();
    return true;
}

bool GDAnimatedGameLights::set_mode_rainbow() noexcept {
    return true;
}

bool GDAnimatedGameLights::set_mode_static(float duration, Color color) noexcept {
    if (!tween.is_valid() || !tween->is_valid()) {
        tween = create_tween();
    }

    set_color_intensity(1.0, color);

    tween->tween_interval(duration);

    Callable callable_off = Callable(this, "set_color_intensity").bind(Color(0, 0, 0));
    tween->tween_method(callable_off, 1.0, 0.0, 0.2);

    return true;
}

void GDAnimatedGameLights::start_animation(float duration, Color color) noexcept {
    if (!connected || !client) {
        gdgamelights::log_error("Tried to start animation without connecting to OpenRGB server!");
        return;
    }

    if (!is_inside_tree()) {
        gdgamelights::log_error("Animations only work if the Node is in the scene tree.");
        return;
    }

    if (!tween.is_valid() || !tween->is_valid()) {
        tween = create_tween();
    }

    if (tween->is_running()) {
        tween->stop(); //stop current animation
    }

    bool success = false;

    switch (mode) {
        case MODE_OFF:
            gdgamelights::log_warn("Mode is set to OFF. Please change the mode to ativate animations.");
            return;
        case MODE_PULSE:
            success = set_mode_pulse(duration, color);
            break;
        case MODE_RAINBOW:
            success = set_mode_rainbow();
            break;
        case MODE_STATIC:
            success = set_mode_static(duration, color);
            break;
        case MODE_CUSTOM:
            //Nothing to do
            break;
        default:
            gdgamelights::log_error("Unsuported mode.");
            return;
    }

    if (success) {
        gdgamelights::log_info("Tween play activated");
        tween->play();
    } else {
        gdgamelights::log_error("Failed to start animation.");
    }

}

void GDAnimatedGameLights::end_animation() noexcept {
    if (!connected || !client) {
        gdgamelights::log_error("Tried to end animation without connecting to OpenRGB server!");
        return;
    }

    if (!tween.is_valid() || !tween->is_valid() || !tween->is_running()) {
        return;
    }

    tween->stop();
}
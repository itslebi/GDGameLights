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
    ClassDB::bind_method(D_METHOD("start_animation", "duration"),
                         &GDAnimatedGameLights::start_animation,
                         DEFVAL(1.0));
    ClassDB::bind_method(D_METHOD("end_animation"),
                         &GDAnimatedGameLights::end_animation);

    ClassDB::bind_method(D_METHOD("set_blue_intensity", "value"), 
                         &GDAnimatedGameLights::set_blue_intensity);
}

// ----------- Methods

void GDAnimatedGameLights::set_mode(AnimationMode p_mode) noexcept {
    mode = p_mode;
}

GDAnimatedGameLights::AnimationMode GDAnimatedGameLights::get_mode() const noexcept {
    return mode;
}

void GDAnimatedGameLights::set_blue_intensity(float value) noexcept {
    gdgamelights::log_info("Running set_blue_intensity");
    set_all_devices_color(Color(0, 0, value));
}

bool GDAnimatedGameLights::set_mode_pulse(float duration) noexcept {
    gdgamelights::log_info("Running set_mode_pulse");
	if (!tween.is_valid() || !tween->is_valid()) {
        tween = create_tween();
    }

    // Animate blue_intensity from 0 → 1
    tween->tween_method(Callable(this, "set_blue_intensity"), 0.0, 1.0, duration / 2.0);
    tween->set_trans(Tween::TransitionType::TRANS_SINE);
    tween->set_ease(Tween::EaseType::EASE_IN_OUT);

    // Animate blue_intensity from 1 → 0
    tween->tween_method(Callable(this, "set_blue_intensity"), 1.0, 0.0, duration / 2.0);
    tween->set_trans(Tween::TransitionType::TRANS_SINE);
    tween->set_ease(Tween::EaseType::EASE_IN_OUT);

    tween->set_loops(); // loop indefinitely
    return true;
}

bool GDAnimatedGameLights::set_mode_rainbow() noexcept {
    return true;
}

bool GDAnimatedGameLights::set_mode_static() noexcept {
    return true;
}

void GDAnimatedGameLights::start_animation(float duration) noexcept {
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
            success = set_mode_pulse(duration);
            break;
        case MODE_RAINBOW:
            success = set_mode_rainbow();
            break;
        case MODE_STATIC:
            success = set_mode_static();
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
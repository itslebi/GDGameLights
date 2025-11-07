#include <GDAnimatedGameLights.hpp>

using namespace godot;

GDAnimatedGameLights::GDAnimatedGameLights()
    : GDGameLights() {}

GDAnimatedGameLights::~GDAnimatedGameLights() {
    if (tween.is_valid()) {
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

    ClassDB::bind_method(D_METHOD("set_mode", "p_mode"), &GDAnimatedGameLights::set_mode);
    ClassDB::bind_method(D_METHOD("get_mode"), &GDAnimatedGameLights::get_mode);

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
    ClassDB::bind_method(D_METHOD("start_animation"),
                         &GDAnimatedGameLights::start_animation);
    ClassDB::bind_method(D_METHOD("end_animation"),
                         &GDAnimatedGameLights::end_animation);
}

// ----------- Methods

void GDAnimatedGameLights::set_mode(AnimationMode p_mode) noexcept {
    mode = p_mode;
}

GDAnimatedGameLights::AnimationMode GDAnimatedGameLights::get_mode() const noexcept {
    return mode;
}


void GDAnimatedGameLights::start_animation() noexcept {
    if (!connected || !client) {
        gdgamelights::log_error("Tried to start animation without connecting to OpenRGB server!");
        return;
    }

    if (!tween->is_valid()) {
        tween = create_tween();
    }

    if (tween->is_running()) {
        tween->stop(); //stop current animation
    }
}

void GDAnimatedGameLights::end_animation() noexcept {
    if (!connected || !client) {
        gdgamelights::log_error("Tried to end animation without connecting to OpenRGB server!");
        return;
    }

    if (!tween->is_valid() && !tween->is_running()) {
        gdgamelights::log_warn("Tried to end animation without starting one.");
        return;
    }
}
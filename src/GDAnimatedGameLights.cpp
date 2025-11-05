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
    BIND_ENUM_CONSTANT(MODE_OFF);
    BIND_ENUM_CONSTANT(MODE_PULSE);
    BIND_ENUM_CONSTANT(MODE_STATIC);
    BIND_ENUM_CONSTANT(MODE_RAINBOW);
    BIND_ENUM_CONSTANT(MODE_CUSTOM);

    ClassDB::bind_method(D_METHOD("set_mode", "p_mode"), &GDAnimatedGameLights::set_mode);
    ClassDB::bind_method(D_METHOD("get_mode"), &GDAnimatedGameLights::get_mode);

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
    if (!tween.is_valid()) {
        tween.instantiate();
    }

    gdgamelights::log_info("Ran without errors!");
}

void GDAnimatedGameLights::end_animation() noexcept {

}
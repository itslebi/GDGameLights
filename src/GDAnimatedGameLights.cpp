#include <GDAnimatedGameLights.hpp>

using namespace godot;

GDAnimatedGameLights::GDAnimatedGameLights()
    : GDGameLights() {}

GDAnimatedGameLights::~GDAnimatedGameLights() {
    if (tween) {
        delete tween;
        tween = nullptr;
    }
}

// ----------- Bind Methods to be used inside GDScript
void GDAnimatedGameLights::_bind_methods() {
    ClassDB::bind_method(D_METHOD("start_animation"),
                         &GDAnimatedGameLights::start_animation);
    ClassDB::bind_method(D_METHOD("end_animation"),
                         &GDAnimatedGameLights::end_animation);
}

// ----------- Methods
void GDAnimatedGameLights::start_animation() noexcept {

}

void GDAnimatedGameLights::end_animation() noexcept {

}
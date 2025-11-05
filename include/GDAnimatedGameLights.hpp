#ifndef GD_ANIMATED_GAMELIGHTS_HPP
#define GD_ANIMATED_GAMELIGHTS_HPP

#include <godot_cpp/classes/tween.hpp>

#include <declarations.hpp>
#include <GDGameLights.hpp>

namespace godot {

class GDAnimatedGameLights : public GDGameLights {
    GDCLASS(GDAnimatedGameLights, GDGameLights);

    public:
        enum AnimationMode {
            MODE_OFF = 0,
            MODE_STATIC,
            MODE_PULSE,
            MODE_RAINBOW,
            MODE_CUSTOM
        };

    private:
        Ref<Tween> tween = nullptr;
        bool animation_running = false;
        AnimationMode mode = MODE_OFF;

    protected:
        static void _bind_methods();

    public:
        GDAnimatedGameLights();
        ~GDAnimatedGameLights();

        void set_mode(AnimationMode p_mode) noexcept;
        AnimationMode get_mode() const noexcept;

        void start_animation() noexcept;
        void end_animation() noexcept;
    };
} //namespace godot

#endif // GD_ANIMATED_GAMELIGHTS_HPP
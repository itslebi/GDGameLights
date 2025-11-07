#ifndef GD_ANIMATED_GAMELIGHTS_HPP
#define GD_ANIMATED_GAMELIGHTS_HPP

#include <godot_cpp/classes/tween.hpp>
#include <godot_cpp/classes/method_tweener.hpp>
#include <godot_cpp/variant/callable.hpp>

#include <declarations.hpp>
#include <GDGameLights.hpp>

namespace godot {

class GDAnimatedGameLights : public GDGameLights {
    GDCLASS(GDAnimatedGameLights, GDGameLights);

    public:
        enum AnimationMode {
            MODE_OFF,
            MODE_STATIC,
            MODE_PULSE,
            MODE_RAINBOW,
            MODE_CUSTOM
        };

    private:
        Ref<Tween> tween = nullptr;
        
        bool set_mode_pulse(float duration = 1.0) noexcept;
        bool set_mode_static() noexcept;
        bool set_mode_rainbow() noexcept;

    protected:
        static void _bind_methods();

    public:
        GDAnimatedGameLights();
        ~GDAnimatedGameLights();

        AnimationMode mode = MODE_OFF;

        void set_mode(AnimationMode p_mode) noexcept;
        AnimationMode get_mode() const noexcept;

        void start_animation(float duration = 1.0) noexcept;
        void end_animation() noexcept;

        void set_blue_intensity(float value) noexcept;
    };
} //namespace godot

#endif // GD_ANIMATED_GAMELIGHTS_HPP
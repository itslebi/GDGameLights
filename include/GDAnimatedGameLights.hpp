#ifndef GD_ANIMATED_GAMELIGHTS_HPP
#define GD_ANIMATED_GAMELIGHTS_HPP

#include <godot_cpp/classes/tween.hpp>


#include <declarations.hpp>
#include <GDGameLights.hpp>

namespace godot {

class GDAnimatedGameLights : public GDGameLights {
    GDCLASS(GDAnimatedGameLights, GDGameLights);

    private:
        Tween* tween = nullptr;
        bool animation_running = false;

    protected:
        static void _bind_methods();

    public:
        GDAnimatedGameLights();
        ~GDAnimatedGameLights();

        void start_animation() noexcept;
        void end_animation() noexcept;
    };
} //namespace godot

#endif // GD_ANIMATED_GAMELIGHTS_HPP
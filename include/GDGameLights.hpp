#ifndef GD_GAMELIGHTS_HPP
#define GD_GAMELIGHTS_HPP

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/color.hpp>

#include <include/OpenRGB/Client.hpp>

namespace godot {

class GDGameLights : public RefCounted {
    GDCLASS(GDGameLights, RefCounted);

    private:
        orgb::Client *client = nullptr;
        bool connected = false;

    protected:
        static void _bind_methods();

    public:
        GDGameLights();
        ~GDGameLights();

        void connect_to_openrgb(String host = "localhost", int port = 6742);
        void set_all_devices_color(Color color);
        void disconnect();
    };

}

#endif // GD_GAMELIGHTS_HPP
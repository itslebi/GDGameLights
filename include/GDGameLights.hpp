#ifndef GD_GAMELIGHTS_HPP
#define GD_GAMELIGHTS_HPP

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/color.hpp>

#include <include/OpenRGB/Client.hpp>
#include <include/OpenRGB/DeviceInfo.hpp>

#include <declarations.hpp>

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

        //Connect to the openrgb server sdk
        void connect_to_openrgb(String host = "localhost", int port = 6742) noexcept;

        //Set the color of the light of all supported devices to color passed
        void set_all_devices_color(Color color) noexcept;

        //Set the mode of all supported devices to color passed
        void set_all_devices_to_direct_mode() noexcept;

        //Disconnect from the openrgb server sdk
        void disconnect() noexcept;
    };
} //namespace godot

#endif // GD_GAMELIGHTS_HPP
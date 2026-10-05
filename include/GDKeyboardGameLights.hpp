#ifndef GD_KEYBOARD_GAMELIGHTS_HPP
#define GD_KEYBOARD_GAMELIGHTS_HPP

#include <unordered_map>

#include <godot_cpp/classes/tween.hpp>
#include <godot_cpp/classes/method_tweener.hpp>
#include <godot_cpp/classes/interval_tweener.hpp>
#include <godot_cpp/variant/callable.hpp>

#include <declarations.hpp>
#include <GDGameLights.hpp>

namespace godot {

class GDKeyboardGameLights : public GDGameLights {
    GDCLASS(GDKeyboardGameLights, GDGameLights);

    public:
        struct KeyMapping {
            orgb::Color color; // current color of this key
            std::vector<uint32_t> led_list; // list with the led indices
        };

    private:
        orgb::DeviceListResult devices = orgb::DeviceListResult();
        const orgb::Device *keyboard = nullptr; // The current keybpard found
        bool is_keyboard_found = false; // Helper var set to true if there is a valid pointer in keyboard variable
        std::unordered_map<int64_t, KeyMapping> key_map;

        // Internal helper function to setup the keyboard mapping
        void setup_keyboard() noexcept;

        std::vector<std::string> getOpenRGBKeyString(Key key) const noexcept;

    protected:
        static void _bind_methods();

    public:
        GDKeyboardGameLights();
        ~GDKeyboardGameLights();

        // Override the connect to set our keyboard after successful connection
        void connect_to_openrgb(String host = "127.0.0.1", int port = 6742) noexcept override;

        // Override disconnect to clear my dangling pointers
        void disconnect() noexcept override;

        // Set keyboard to direct mode
        void set_all_devices_to_direct_mode() noexcept override;

        // Set the color of keys
        void set_key_color_bulk(Color color, const TypedArray<int64_t> &keys) const noexcept;
    };
} //namespace godot

#endif // GD_KEYBOARD_GAMELIGHTS_HPP
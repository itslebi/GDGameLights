#ifndef GD_KEYBOARD_GAMELIGHTS_HPP
#define GD_KEYBOARD_GAMELIGHTS_HPP

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
        // Enum for easier key identification inside the godot editor
        enum UniversalKey {
            KEY_ESC, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12,
            KEY_TILDE, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0, KEY_MINUS, KEY_EQUAL, KEY_BACKSPACE,
            KEY_TAB, KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T, KEY_Y, KEY_U, KEY_I, KEY_O, KEY_P, KEY_LEFT_BRACKET, KEY_RIGHT_BRACKET, KEY_ENTER,
            KEY_CAPS_LOCK, KEY_A, KEY_S, KEY_D, KEY_F, KEY_G, KEY_H, KEY_J, KEY_K, KEY_L, KEY_SEMICOLON, KEY_APOSTROPHE, KEY_BACKSLASH,
            KEY_LEFT_SHIFT, KEY_NON_US_BACKSLASH, KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_N, KEY_M, KEY_COMMA, KEY_PERIOD, KEY_FORWARD_SLASH, KEY_RIGHT_SHIFT,
            KEY_LEFT_CONTROL, KEY_LEFT_WINDOWS, KEY_LEFT_ALT, KEY_SPACE, KEY_RIGHT_ALT, KEY_RIGHT_WINDOWS, KEY_MENU, KEY_RIGHT_CONTROL,
            KEY_PRINTSCREEN, KEY_SCROLL_LOCK, KEY_PAUSE, KEY_INSERT, KEY_HOME, KEY_PAGE_UP, KEY_DELETE, KEY_END, KEY_PAGE_DOWN,
            KEY_UP_ARROW, KEY_LEFT_ARROW, KEY_DOWN_ARROW, KEY_RIGHT_ARROW,
            TOTAL_KEYS // Total tracking bound
        };

        const orgb::Device *keyboard = nullptr; // The current keybpard found
        int index_map[TOTAL_KEYS]; // Fast hardware index lookup table
        bool is_keyboard_found = false; // Helper var set to true if there is a valid pointer in keyboard variable

    private:
        // Internal function to convert our handy enum for godot editor to the real string names used by ORGB SDK
        String getOpenRGBKeyString(UniversalKey key) const noexcept;

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
    };
} //namespace godot

#endif // GD_KEYBOARD_GAMELIGHTS_HPP
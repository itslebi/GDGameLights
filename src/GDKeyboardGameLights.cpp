#include <GDKeyboardGameLights.hpp>

using namespace godot;

GDKeyboardGameLights::GDKeyboardGameLights()
    : GDGameLights() {
}

GDKeyboardGameLights::~GDKeyboardGameLights() {
}

// ----------- Bind Methods to be used inside GDScript
VARIANT_ENUM_CAST(GDKeyboardGameLights::UniversalKey);

void GDKeyboardGameLights::_bind_methods() {
    //Bind enum
    BIND_ENUM_CONSTANT(KEY_ESC);
    BIND_ENUM_CONSTANT(KEY_F1);
    BIND_ENUM_CONSTANT(KEY_F2);
    BIND_ENUM_CONSTANT(KEY_F3);

    //Methods
    ClassDB::bind_method(D_METHOD("getOpenRGBKeyString", "key"),
                         &GDKeyboardGameLights::getOpenRGBKeyString);
}

// ----------- Methods
void GDKeyboardGameLights::connect_to_openrgb(String host, int port) noexcept {
    GDGameLights::connect_to_openrgb(host, port);
    if (connected) {
        orgb::DeviceListResult devices = this->client->requestDeviceList();
        if (devices.status == orgb::RequestStatus::Success) {
            keyboard = devices.devices.find(orgb::DeviceType::Keyboard);

            if (this->keyboard == nullptr) {
                gdgamelights::log_error("Failed to find valid keyboard.");
            }
            is_keyboard_found = true;
        }
    }
}

void GDKeyboardGameLights::disconnect() noexcept {
    GDGameLights::disconnect();

    if (!client) {
        is_keyboard_found = false;
        keyboard = nullptr;
    }
}

void GDKeyboardGameLights::set_all_devices_to_direct_mode() noexcept {
    if (!connected || !client) {
        gdgamelights::log_error("Not connected to OpenRGB server!");
        return;
    }

    if (!is_keyboard_found || !keyboard) {
        gdgamelights::log_error("No keyboard found!");
        return;
    }

    if (direct) {
        gdgamelights::log_info("Already on direct mode.");
        return;
    }

    if (keyboard->modes.empty()) {
        gdgamelights::log_info("Device " + godot::String(keyboard->name.c_str()) + " has no modes. Skipping.");
        return;
    }

    const orgb::Mode *modeToUse = nullptr;
    const orgb::Mode *fallbackMode = nullptr;

    // Find the mode
    for (const auto &mode : keyboard->modes) {
        // Look at the OpenRGB SDK headers for the flag definition (usually MODE_FLAG_HAS_SPEED)
        // Assuming standard OpenRGB SDK bitwise rules:
        bool has_speed = (mode.flags & (1 << 0)); // Or the exact SDK enum for HasSpeed

        if (mode.name == "Direct" || (mode.color_mode == orgb::ColorMode::PerLed && !has_speed)) { 
            // Some devices do not have the correct name mapping for direct mode!
            modeToUse = &mode;
            break;
        }
        if (!fallbackMode && mode.name != "Off") {
            fallbackMode = &mode;
        }
    }

    if (!modeToUse) { // Use fallback
        modeToUse = fallbackMode;
    }

    // Attempt to change the mode
    orgb::RequestStatus status = client->changeMode(*keyboard, *modeToUse);
    if (status != orgb::RequestStatus::Success) {
        gdgamelights::log_error("Failed to set mode for device: " + String(keyboard->name.c_str()));
    } else {
        gdgamelights::log_info("Device " + String(keyboard->name.c_str()) + " mode set to " + String(modeToUse->name.c_str()));
    }

    direct = true;

}

String GDKeyboardGameLights::getOpenRGBKeyString(UniversalKey key) const noexcept {
    switch (key) {
        case KEY_ESC: return "Key: Escape";
        case KEY_TILDE: return "Key: `";
        case KEY_MINUS: return "Key: -";
        case KEY_EQUAL: return "Key: =";
        case KEY_LEFT_BRACKET: return "Key: [";
        case KEY_RIGHT_BRACKET: return "Key: ]";
        case KEY_SEMICOLON: return "Key: ;";
        case KEY_APOSTROPHE: return "Key: '";
        case KEY_BACKSLASH: return "Key: \\";
        case KEY_COMMA: return "Key: ,";
        case KEY_PERIOD: return "Key: .";
        case KEY_FORWARD_SLASH: return "Key: /";
        case KEY_NON_US_BACKSLASH: return "Key: Non-US \\"; // Critical for European ISO boards
        case KEY_SPACE: return "Key: Space";
        case KEY_ENTER: return "Key: Enter";
        case KEY_BACKSPACE: return "Key: Backspace";
        case KEY_TAB: return "Key: Tab";
        case KEY_CAPS_LOCK: return "Key: Caps Lock";
        case KEY_LEFT_SHIFT: return "Key: Left Shift";
        case KEY_RIGHT_SHIFT: return "Key: Right Shift";
        case KEY_LEFT_CONTROL: return "Key: Left Control";
        case KEY_RIGHT_CONTROL: return "Key: Right Control";
        case KEY_LEFT_ALT: return "Key: Left Alt";
        case KEY_RIGHT_ALT: return "Key: Right Alt";
        case KEY_LEFT_WINDOWS: return "Key: Left Window";
        case KEY_RIGHT_WINDOWS: return "Key: Right Window";
        case KEY_MENU: return "Key: Menu";
        case KEY_UP_ARROW: return "Key: Up Arrow";
        case KEY_DOWN_ARROW: return "Key: Down Arrow";
        case KEY_LEFT_ARROW: return "Key: Left Arrow";
        case KEY_RIGHT_ARROW: return "Key: Right Arrow";
        
        // Single characters/numbers default directly to OpenRGB standard formatting "Key: X"
        default: {
            if (key >= KEY_F1 && key <= KEY_F12) {
                return ("Key: F" + std::to_string(key - KEY_F1 + 1)).c_str();
            }
            if (key >= KEY_1 && key <= KEY_9) {
                return ("Key: " + std::to_string(key - KEY_1 + 1)).c_str();
            }
            if (key == KEY_0) return "Key: 0";
            
            // Handle letters A-Z mapping
            if (key >= KEY_Q && key <= KEY_P) return (std::string("Key: ") + (char)('Q' + (key - KEY_Q))).c_str();
            if (key >= KEY_A && key <= KEY_L) return (std::string("Key: ") + (char)('A' + (key - KEY_A))).c_str();
            if (key >= KEY_Z && key <= KEY_M) return (std::string("Key: ") + (char)('Z' + (key - KEY_Z))).c_str();
            
            return "Unknown";
        }
    }
}
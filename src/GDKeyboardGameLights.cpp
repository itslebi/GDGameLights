#include <GDKeyboardGameLights.hpp>

using namespace godot;

GDKeyboardGameLights::GDKeyboardGameLights()
    : GDGameLights() {
}

GDKeyboardGameLights::~GDKeyboardGameLights() {
}

void GDKeyboardGameLights::_bind_methods() {
    //Methods
    ClassDB::bind_method(D_METHOD("set_key_color_bulk", "color", "keys"),
                         &GDKeyboardGameLights::set_key_color_bulk);
}

// ----------- Methods
void GDKeyboardGameLights::set_key_color_bulk(Color color, const TypedArray<int64_t> &keys) const noexcept {
    if (!connected || !client) {
        gdgamelights::log_error("Not connected to OpenRGB server!");
        return;
    }

    if (!keyboard) {
        gdgamelights::log_error("No keyboard found!");
        return;
    }

    // One color entry for every LED on the device.
    std::vector<orgb::Color> colors(keyboard->leds.size(), orgb::Color::Black);

    const orgb::Color col = convert_color(color);

    for (int64_t i = 0; i < keys.size(); ++i) {
        const int64_t key_value = keys[i].operator int64_t();

        auto it = key_map.find(key_value);

        if (it == key_map.end()) {
            gdgamelights::log_error("Godot key has no OpenRGB LED mapping: " + String::num_int64(key_value));
            continue;
        }

        const KeyMapping &mapping = it->second;

        for (const uint32_t led_idx : mapping.led_list) {
            if (led_idx >= colors.size()) {
                //gdgamelights::log_error("LED index out of range: " + String::num_int64(static_cast<int64_t>(led_idx)));
                continue;
            }

            colors[led_idx] = col;
        }
    }


    const orgb::RequestStatus status = client->setLEDColorBulk(*keyboard, colors);

    if (status != orgb::RequestStatus::Success) {
        gdgamelights::log_error("setLEDColorBulk failed.");
    }
}

void GDKeyboardGameLights::connect_to_openrgb(String host, int port) noexcept {
    GDGameLights::connect_to_openrgb(host, port);
    if (connected) {
        devices = this->client->requestDeviceList();
        if (devices.status == orgb::RequestStatus::Success) {
            keyboard = devices.devices.find(orgb::DeviceType::Keyboard);

            if (this->keyboard == nullptr) {
                gdgamelights::log_error("Failed to find valid keyboard.");
            }
            setup_keyboard();
        } else {
            gdgamelights::log_error("Failed to find valid keyboard.");
        }
    }
}

void GDKeyboardGameLights::setup_keyboard() noexcept {
    key_map.clear();

    if (!keyboard) {
        is_keyboard_found = false;
        return;
    }

    is_keyboard_found = true;

    gdgamelights::log_info(String::num_uint64(keyboard->leds.size()));


    auto process_key = [this](Key key) {
        const std::vector<std::string> openrgb_names =
            getOpenRGBKeyString(key);

        if (openrgb_names.empty()) {
            return;
        }

        auto &[color, led_indices] =
            key_map[static_cast<int64_t>(key)];

        for (const auto &led : keyboard->leds) {
            const auto name_it = std::find(
                openrgb_names.begin(),
                openrgb_names.end(),
                led.name
            );

            if (name_it != openrgb_names.end()) {
                led_indices.push_back(led.idx);
            }
        }

        if (led_indices.empty()) {
            gdgamelights::log_info(godot::String("Failed to map key: ") + godot::String::num_int64(static_cast<int64_t>(key)));

            key_map.erase(static_cast<int64_t>(key));
        }
    };

    auto process_range = [&process_key](Key first, Key last) {
        for (int64_t value = static_cast<int64_t>(first);
            value <= static_cast<int64_t>(last);
            ++value) {
            process_key(static_cast<Key>(value));
        }
    };

    // Special keys.
    process_key(KEY_SPECIAL); // Fn Key
    process_range(KEY_ESCAPE, KEY_F35);

    // Other special keys.
    process_range(KEY_MENU, KEY_JIS_KANA);
    process_range(KEY_HYPER, KEY_HYPER);
    process_range(KEY_HELP, KEY_HELP);
    process_range(KEY_BACK, KEY_FORWARD);
    process_range(KEY_STOP, KEY_REFRESH);
    process_range(KEY_VOLUMEDOWN, KEY_VOLUMEUP);
    process_range(KEY_MEDIAPLAY, KEY_JIS_KANA);

    // Keypad.
    process_range(KEY_KP_MULTIPLY, KEY_KP_9);

    // Printable keys.
    process_range(KEY_SPACE, KEY_ASCIITILDE);

    // Individual keys.
    process_key(KEY_YEN);
    process_key(KEY_SECTION);

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


std::vector<std::string> GDKeyboardGameLights::getOpenRGBKeyString(Key key) const noexcept {
    switch (key) {
        case KEY_NONE:
        case KEY_UNKNOWN:
            return {};

        // Special is Fn key
        case KEY_SPECIAL:
            return {
                "Key: Fn",
                "Key: Function",
                "Key: Left Fn",
                "Key: Right Fn"
            };

        // ---------------------------------------------------------------------
        // Special keys
        // ---------------------------------------------------------------------

        case KEY_ESCAPE:
            return {"Key: Escape"};

        case KEY_TAB:
        case KEY_BACKTAB:
            return {"Key: Tab"};

        case KEY_BACKSPACE:
            return {"Key: Backspace"};

        case KEY_ENTER:
            return {
                "Key: Enter",
                "Key: Return"
            };

        case KEY_KP_ENTER:
            return {
                "Key: Number Pad Enter",
                "Key: NumPad Enter",
                "Key: Keypad Enter"
            };

        case KEY_INSERT:
            return {"Key: Insert"};

        case KEY_DELETE:
            return {"Key: Delete"};

        case KEY_PAUSE:
            return {
                "Key: Pause/Break",
                "Key: Pause"
            };

        case KEY_PRINT:
            return {
                "Key: Print Screen",
                "Key: PrintScreen"
            };

        case KEY_SYSREQ:
            return {
                "Key: SysRq",
                "Key: Print Screen",
                "Key: PrintScreen"
            };

        case KEY_CLEAR:
            return {"Key: Clear"};

        case KEY_HOME:
            return {"Key: Home"};

        case KEY_END:
            return {"Key: End"};

        case KEY_LEFT:
            return {
                "Key: Left Arrow",
                "Key: Left"
            };

        case KEY_UP:
            return {
                "Key: Up Arrow",
                "Key: Up"
            };

        case KEY_RIGHT:
            return {
                "Key: Right Arrow",
                "Key: Right"
            };

        case KEY_DOWN:
            return {
                "Key: Down Arrow",
                "Key: Down"
            };

        case KEY_PAGEUP:
            return {
                "Key: Page Up",
                "Key: PageUp"
            };

        case KEY_PAGEDOWN:
            return {
                "Key: Page Down",
                "Key: PageDown"
            };

        case KEY_SHIFT:
            return {
                "Key: Left Shift",
                "Key: Right Shift"
            };

        case KEY_CTRL:
            return {
                "Key: Left Control",
                "Key: Right Control"
            };

        case KEY_ALT:
            return {
                "Key: Left Alt",
                "Key: Right Alt"
            };

        case KEY_META:
            return {
                "Key: Left Windows",
                "Key: Right Windows"
            };

        case KEY_CAPSLOCK:
            return {"Key: Caps Lock"};

        case KEY_NUMLOCK:
            return {
                "Key: Num Lock",
                "Key: NumLock"
            };

        case KEY_SCROLLLOCK:
            return {
                "Key: Scroll Lock",
                "Key: ScrollLock"
            };

        // ---------------------------------------------------------------------
        // Function keys
        // ---------------------------------------------------------------------

        case KEY_F1:
        case KEY_F2:
        case KEY_F3:
        case KEY_F4:
        case KEY_F5:
        case KEY_F6:
        case KEY_F7:
        case KEY_F8:
        case KEY_F9:
        case KEY_F10:
        case KEY_F11:
        case KEY_F12:
        case KEY_F13:
        case KEY_F14:
        case KEY_F15:
        case KEY_F16:
        case KEY_F17:
        case KEY_F18:
        case KEY_F19:
        case KEY_F20:
        case KEY_F21:
        case KEY_F22:
        case KEY_F23:
        case KEY_F24:
        case KEY_F25:
        case KEY_F26:
        case KEY_F27:
        case KEY_F28:
        case KEY_F29:
        case KEY_F30:
        case KEY_F31:
        case KEY_F32:
        case KEY_F33:
        case KEY_F34:
        case KEY_F35:
            return {
                "Key: F" +
                std::to_string(
                    static_cast<int>(key) -
                    static_cast<int>(KEY_F1) + 1
                )
            };

        // ---------------------------------------------------------------------
        // System / media keys
        // ---------------------------------------------------------------------

        case KEY_MENU:
            return {
                "Key: Menu",
                "Key: Application",
                "Key: Apps"
            };

        case KEY_HYPER:
            return {"Key: Hyper"};

        case KEY_HELP:
            return {"Key: Help"};

        case KEY_BACK:
            return {"Key: Back"};

        case KEY_FORWARD:
            return {"Key: Forward"};

        case KEY_STOP:
            return {"Key: Stop"};

        case KEY_REFRESH:
            return {
                "Key: Refresh",
                "Key: Reload"
            };

        case KEY_VOLUMEDOWN:
            return {
                "Key: Media Volume -",
                "Key: Volume Down"
            };

        case KEY_VOLUMEMUTE:
            return {
                "Key: Media Mute",
                "Key: Mute"
            };

        case KEY_VOLUMEUP:
            return {
                "Key: Media Volume +",
                "Key: Volume Up"
            };

        case KEY_MEDIAPLAY:
            return {
                "Key: Media Play/Pause",
                "Key: Play/Pause",
                "Key: Media Play"
            };

        case KEY_MEDIASTOP:
            return {
                "Key: Media Stop",
                "Key: Stop Media"
            };

        case KEY_MEDIAPREVIOUS:
            return {
                "Key: Media Previous",
                "Key: Previous Track"
            };

        case KEY_MEDIANEXT:
            return {
                "Key: Media Next",
                "Key: Next Track"
            };

        case KEY_MEDIARECORD:
            return {
                "Key: Media Record",
                "Key: Record"
            };

        case KEY_HOMEPAGE:
            return {"Key: Home Page"};

        case KEY_FAVORITES:
            return {"Key: Favorites"};

        case KEY_SEARCH:
            return {"Key: Search"};

        case KEY_STANDBY:
            return {"Key: Standby"};

        case KEY_OPENURL:
            return {"Key: Open URL"};

        case KEY_LAUNCHMAIL:
            return {"Key: Launch Mail"};

        case KEY_LAUNCHMEDIA:
            return {"Key: Launch Media"};

        case KEY_LAUNCH0:
            return {"Key: Launch 0"};

        case KEY_LAUNCH1:
            return {"Key: Launch 1"};

        case KEY_LAUNCH2:
            return {"Key: Launch 2"};

        case KEY_LAUNCH3:
            return {"Key: Launch 3"};

        case KEY_LAUNCH4:
            return {"Key: Launch 4"};

        case KEY_LAUNCH5:
            return {"Key: Launch 5"};

        case KEY_LAUNCH6:
            return {"Key: Launch 6"};

        case KEY_LAUNCH7:
            return {"Key: Launch 7"};

        case KEY_LAUNCH8:
            return {"Key: Launch 8"};

        case KEY_LAUNCH9:
            return {"Key: Launch 9"};

        case KEY_LAUNCHA:
            return {"Key: Launch A"};

        case KEY_LAUNCHB:
            return {"Key: Launch B"};

        case KEY_LAUNCHC:
            return {"Key: Launch C"};

        case KEY_LAUNCHD:
            return {"Key: Launch D"};

        case KEY_LAUNCHE:
            return {"Key: Launch E"};

        case KEY_LAUNCHF:
            return {"Key: Launch F"};

        case KEY_GLOBE:
            return {"Key: Globe"};

        case KEY_KEYBOARD:
            return {"Key: Keyboard"};

        case KEY_JIS_EISU:
            return {"Key: JIS Eisu"};

        case KEY_JIS_KANA:
            return {"Key: JIS Kana"};

        // ---------------------------------------------------------------------
        // Keypad
        // ---------------------------------------------------------------------

        case KEY_KP_MULTIPLY:
            return {"Key: Number Pad *"};

        case KEY_KP_DIVIDE:
            return {"Key: Number Pad /"};

        case KEY_KP_SUBTRACT:
            return {"Key: Number Pad -"};

        case KEY_KP_PERIOD:
            return {"Key: Number Pad ."};

        case KEY_KP_ADD:
            return {"Key: Number Pad +"};

        case KEY_KP_0:
            return {"Key: Number Pad 0"};

        case KEY_KP_1:
            return {"Key: Number Pad 1"};

        case KEY_KP_2:
            return {"Key: Number Pad 2"};

        case KEY_KP_3:
            return {"Key: Number Pad 3"};

        case KEY_KP_4:
            return {"Key: Number Pad 4"};

        case KEY_KP_5:
            return {"Key: Number Pad 5"};

        case KEY_KP_6:
            return {"Key: Number Pad 6"};

        case KEY_KP_7:
            return {"Key: Number Pad 7"};

        case KEY_KP_8:
            return {"Key: Number Pad 8"};

        case KEY_KP_9:
            return {"Key: Number Pad 9"};

        // ---------------------------------------------------------------------
        // Space
        // ---------------------------------------------------------------------

        case KEY_SPACE:
            return {"Key: Space"};

        // ---------------------------------------------------------------------
        // Number row
        //
        // Godot distinguishes the shifted character from the physical key.
        // OpenRGB identifies the physical key, so:
        //
        //   ! -> 1
        //   @ -> 2
        //   # -> 3
        //   etc.
        // ---------------------------------------------------------------------

        case KEY_1:
        case KEY_EXCLAM:
            return {"Key: 1"};

        case KEY_2:
        case KEY_QUOTEDBL:
        case KEY_AT:
            return {"Key: 2"};

        case KEY_3:
        case KEY_NUMBERSIGN:
            return {"Key: 3"};

        case KEY_4:
        case KEY_DOLLAR:
            return {"Key: 4"};

        case KEY_5:
        case KEY_PERCENT:
            return {"Key: 5"};

        case KEY_6:
        case KEY_ASCIICIRCUM:
            return {"Key: 6"};

        case KEY_7:
        case KEY_AMPERSAND:
            return {"Key: 7"};

        case KEY_8:
        case KEY_ASTERISK:
            return {"Key: 8"};

        case KEY_9:
        case KEY_PARENLEFT:
            return {"Key: 9"};

        case KEY_0:
        case KEY_PARENRIGHT:
            return {"Key: 0"};

        // ---------------------------------------------------------------------
        // Punctuation
        // ---------------------------------------------------------------------

        case KEY_MINUS:
        case KEY_UNDERSCORE:
            return {"Key: -"};

        case KEY_EQUAL:
        case KEY_PLUS:
            return {"Key: ="};

        case KEY_COMMA:
        case KEY_LESS:
            return {"Key: ,"};

        case KEY_PERIOD:
        case KEY_GREATER:
            return {"Key: ."};

        case KEY_SLASH:
        case KEY_QUESTION:
            return {"Key: /"};

        case KEY_SEMICOLON:
        case KEY_COLON:
            return {"Key: ;"};

        case KEY_APOSTROPHE:
            return {"Key: '"};

        case KEY_QUOTELEFT:
        case KEY_ASCIITILDE:
            return {"Key: `"};

        case KEY_BRACKETLEFT:
        case KEY_BRACELEFT:
            return {"Key: ["};

        case KEY_BRACKETRIGHT:
        case KEY_BRACERIGHT:
            return {"Key: ]"};

        case KEY_BACKSLASH:
        case KEY_BAR:
            return {
                "Key: \\",
                "Key: \\ (ANSI)",
                "Key: \\ (ISO)"
            };

        // ---------------------------------------------------------------------
        // Alphabetic keys
        // ---------------------------------------------------------------------

        case KEY_A:
            return {"Key: A"};

        case KEY_B:
            return {"Key: B"};

        case KEY_C:
            return {"Key: C"};

        case KEY_D:
            return {"Key: D"};

        case KEY_E:
            return {"Key: E"};

        case KEY_F:
            return {"Key: F"};

        case KEY_G:
            return {"Key: G"};

        case KEY_H:
            return {"Key: H"};

        case KEY_I:
            return {"Key: I"};

        case KEY_J:
            return {"Key: J"};

        case KEY_K:
            return {"Key: K"};

        case KEY_L:
            return {"Key: L"};

        case KEY_M:
            return {"Key: M"};

        case KEY_N:
            return {"Key: N"};

        case KEY_O:
            return {"Key: O"};

        case KEY_P:
            return {"Key: P"};

        case KEY_Q:
            return {"Key: Q"};

        case KEY_R:
            return {"Key: R"};

        case KEY_S:
            return {"Key: S"};

        case KEY_T:
            return {"Key: T"};

        case KEY_U:
            return {"Key: U"};

        case KEY_V:
            return {"Key: V"};

        case KEY_W:
            return {"Key: W"};

        case KEY_X:
            return {"Key: X"};

        case KEY_Y:
            return {"Key: Y"};

        case KEY_Z:
            return {"Key: Z"};

        // ---------------------------------------------------------------------
        // Layout-specific keys
        // ---------------------------------------------------------------------

        case KEY_YEN:
            return {
                "Key: ¥",
                "Key: Yen"
            };

        case KEY_SECTION:
            return {
                "Key: Section",
                "Key: §",
                "Key: < >"
            };

        default:
            return {};
    }
}

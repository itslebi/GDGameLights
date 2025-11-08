#include "GDGameLights.hpp"

using namespace godot;

GDGameLights::GDGameLights() {}

GDGameLights::~GDGameLights() {
    disconnect();
}

// ----------- Bind Methods to be used inside GDScript
void GDGameLights::_bind_methods() {
    ClassDB::bind_method(D_METHOD("connect_to_openrgb", "host", "port"),
                         &GDGameLights::connect_to_openrgb, 
                         DEFVAL("127.0.0.1"), 
                         DEFVAL(6742));
    ClassDB::bind_method(D_METHOD("set_all_devices_color", "color"),
                         &GDGameLights::set_all_devices_color);
    ClassDB::bind_method(D_METHOD("set_all_devices_to_direct_mode"),
                         &GDGameLights::set_all_devices_to_direct_mode);
    ClassDB::bind_method(D_METHOD("disconnect"),
                         &GDGameLights::disconnect);
}

// ----------- Methods
void GDGameLights::connect_to_openrgb(String host, int port) noexcept {
    if (connected) {
        gdgamelights::log_info("Already connected to OpenRGB.");
        return;
    }

    if (!gdgamelights::isValidIPv4(host.utf8().get_data())) {
        gdgamelights::log_error("Tried connecting to invalid port");
        return;
    }

    gdgamelights::log_info("Connecting to OpenRGB at host: " + host + ", port: " + String::num_int64(port));

    client = memnew(orgb::Client());
    client->connect(host.utf8().get_data(), port);

    if (!client->isConnected()) {
        gdgamelights::log_error("Failed to connect to OpenRGB at " + host + ":" + String::num_int64(port));
        memdelete(client);
        client = nullptr;
        connected = false;
        return;
    }

    connected = true;
    gdgamelights::log_info("Connected to OpenRGB server at " + host + ":" + String::num_int64(port));
}

void GDGameLights::set_all_devices_color(Color color) noexcept {
    if (!connected || !client) {
        gdgamelights::log_error("Not connected to OpenRGB server!");
        return;
    }

    // Convert Godot Color (0.0–1.0 range) to 8-bit RGB
    orgb::Color col{
        static_cast<uint8_t>(color.r * 255),
        static_cast<uint8_t>(color.g * 255),
        static_cast<uint8_t>(color.b * 255)
    };

    orgb::DeviceListResult deviceList = client->requestDeviceList();

    if (deviceList.status != orgb::RequestStatus::Success) {
        gdgamelights::log_error("Failed to get device list from OpenRGB.");
        return;
    }

    // Set the same color for all devices
    for (const auto &device : deviceList.devices) {
        orgb::RequestStatus status = client->setDeviceColor(device, col);
        if (status != orgb::RequestStatus::Success) {
            gdgamelights::log_error("Failed to set color for device: " + godot::String(device.name.c_str()));
        }
    }

    gdgamelights::log_info("Set all devices to color (" + String::num_int64((int)col.r) + ", " + String::num_int64((int)col.g) + ", " + String::num_int64((int)col.b) + ")");
}

void GDGameLights::set_all_devices_to_direct_mode() noexcept {
    if (!connected || !client) {
        gdgamelights::log_error("Not connected to OpenRGB server!");
        return;
    }

    if (direct) {
        gdgamelights::log_info("Already on direct mode.");
        return;
    }

    orgb::DeviceListResult deviceList = client->requestDeviceList();

    if (deviceList.status != orgb::RequestStatus::Success) {
        gdgamelights::log_error("Failed to get device list from OpenRGB.");
        return;
    }

    for (auto &device : deviceList.devices) {
        if (device.modes.empty()) {
            gdgamelights::log_info("Device " + godot::String(device.name.c_str()) + " has no modes. Skipping.");
            continue;
        }

        orgb::Mode *modeToUse = nullptr;
        orgb::Mode *fallbackMode = nullptr;

        // Find the mode
        for (auto &mode : device.modes) {
            if (mode.name == "Direct") {
                orgb::Mode modeCopy = mode;
                modeToUse = &modeCopy;
                break;
            }
            if (!fallbackMode && mode.name != "Off") {
                orgb::Mode modeCopy = mode;
                fallbackMode = &modeCopy;
            }
        }

        if (!modeToUse) { // Use fallback
            modeToUse = fallbackMode;
        }

        // Attempt to change the mode
        orgb::RequestStatus status = client->changeMode(device, *modeToUse);
        if (status != orgb::RequestStatus::Success) {
            gdgamelights::log_error("Failed to set mode for device: " + String(device.name.c_str()));
        } else {
            gdgamelights::log_info("Device " + String(device.name.c_str()) + " mode set to" + String(modeToUse->name.c_str()));
        }
    }

    direct = true;
}



void GDGameLights::disconnect() noexcept {
    if (client) {
        if (connected) {
            client->disconnect();
            gdgamelights::log_info("Disconnected from OpenRGB.");
        }

        memdelete(client);
        client = nullptr;
        connected = false;
    }
}

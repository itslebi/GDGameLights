#include "GDGameLights.hpp"

#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

GDGameLights::GDGameLights() {}

GDGameLights::~GDGameLights() {
    disconnect();
}

void GDGameLights::_bind_methods() {
    ClassDB::bind_method(D_METHOD("connect_to_openrgb", "host", "port"),
                         &GDGameLights::connect_to_openrgb, DEFVAL("localhost"), DEFVAL(6742));
    ClassDB::bind_method(D_METHOD("set_all_devices_color", "color"),
                         &GDGameLights::set_all_devices_color);
    ClassDB::bind_method(D_METHOD("disconnect"),
                         &GDGameLights::disconnect);
}

void GDGameLights::connect_to_openrgb(String host, int port) {
    if (connected) {
        UtilityFunctions::print("Already connected to OpenRGB.");
        return;
    }

    UtilityFunctions::print("Connecting to OpenRGB at host: " + host + ", port: " + String::num_int64(port));

    client = memnew(orgb::Client());
    client->connect(host.utf8().get_data(), port);

    if (!client->isConnected()) {
        UtilityFunctions::printerr("Failed to connect to OpenRGB at " + host + ":" + String::num_int64(port));
        memdelete(client);
        client = nullptr;
        connected = false;
        return;
    }

    connected = true;
    UtilityFunctions::print("Connected to OpenRGB server at " + host + ":" + String::num_int64(port));
}

void GDGameLights::set_all_devices_color(Color color) {
    if (!connected || !client) {
        UtilityFunctions::printerr("Not connected to OpenRGB server!");
        return;
    }

    // Convert Godot Color (0.0–1.0 range) to 8-bit RGB
    orgb::Color col{
        static_cast<uint8_t>(color.r * 255),
        static_cast<uint8_t>(color.g * 255),
        static_cast<uint8_t>(color.b * 255)
    };

    // Get all devices
    orgb::DeviceListResult deviceList = client->requestDeviceList();

    if (deviceList.status != orgb::RequestStatus::Success) {
        UtilityFunctions::printerr("Failed to get device list from OpenRGB.");
        return;
    }

    // Set the same color for all devices
    for (const auto &device : deviceList.devices) {
        orgb::RequestStatus status = client->setDeviceColor(device, col);
        if (status != orgb::RequestStatus::Success) {
            UtilityFunctions::printerr(
                godot::String("Failed to set color for device: ") + godot::String(device.name.c_str())
            );
        }
    }

    UtilityFunctions::print("Set all devices to color (", (int)col.r, ", ", (int)col.g, ", ", (int)col.b, ")");
}

void GDGameLights::disconnect() {
    if (client) {
        if (connected) {
            client->disconnect();
            UtilityFunctions::print("Disconnected from OpenRGB.");
        }

        memdelete(client);
        client = nullptr;
        connected = false;
    }
}

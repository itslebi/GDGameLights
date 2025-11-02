#ifndef GD_DECLARATIONS_HPP
#define GD_DECLARATIONS_HPP

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace gdgamelights {
    // Error log without crashing execution that does not appear in the debugger
    inline void log_error(const godot::String &msg) noexcept {
        godot::UtilityFunctions::printerr("[GDGameLights] " + msg);
    }

    // Error log without crashing execution that appears in the debugger
    inline void track_error(const godot::String &msg) noexcept {
        godot::UtilityFunctions::push_error("[GDGameLights] " + msg);
    }

    // Error log that crashes execution
    inline void crash_error(const godot::String &msg) {
        ERR_FAIL_MSG("[GDGameLights] " + msg);
    }

    // Warning log always shown
    inline void log_warn(const godot::String &msg) noexcept  {
        godot::UtilityFunctions::push_warning("[GDGameLights] " + msg);
    }

    // Info log shown only in editor
    inline void log_info(const godot::String &msg) noexcept {
        #ifdef IN_EDITOR
            godot::UtilityFunctions::print("[GDGameLights] " + msg);
        #endif
    }
} //namespace gdgamelights

#endif //GD_DECLARATIONS_HPP
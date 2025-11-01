import sys

if not ARGUMENTS.get("target"):
    ARGUMENTS["target"] = "template_debug"

env = SConscript("godot-cpp/SConstruct")

# --- Compiler-specific flags ---
if env["CC"] == "cl" or env["CXX"] == "cl":
    env.Append(CXXFLAGS=["/EHsc"])  # enable exceptions for MSVC
else:
    env.Append(CXXFLAGS=["-std=c++20"])  # GCC/Clang standard

# --- Shared preprocessor definitions ---
env.Append(CPPDEFINES=[("critical_error", "CRITICAL_ERROR")])

# --- Platform-specific linkage ---
is_windows = sys.platform.startswith("win")   # 'win32' on Windows
is_linux   = sys.platform.startswith("linux") # Linux / WSL
is_mac     = sys.platform.startswith("darwin") # macOS

if is_windows:
    env.Append(LIBS=["Ws2_32"])
    env.Append(LINKFLAGS=["/DEFAULTLIB:Ws2_32.lib"])
    print("Windows detected")

elif is_mac:
    env.Append(FRAMEWORKS=["CoreFoundation", "CoreServices"])
    print("macOS detected")

elif is_linux:
    env.Append(LIBS=["pthread"])
    print("Linux/WSL detected")

# --- Include paths ---
env.Append(CPPPATH=[
    "src/",
    "src/headers",
    "openRGB",
    "openRGB/include",
    "openRGB/src",
    "openRGB/external",
])

# --- Source files ---
sources = (
    # GDGameLights Source
    Glob("src/*.cpp") +
    Glob("src/headers/*.cpp") +
    # Godot CPP Source
    Glob("godot-cpp/include/src/*.cpp") +
    # OpenRGB SDK
    Glob("openRGB/src/*.cpp") +
    Glob("openRGB/external/CppUtils-Essential/*.cpp") +
    Glob("openRGB/external/CppUtils-Network/*.cpp")
)

# --- Build shared library ---
library = env.SharedLibrary(
    "demo/bin/GDGameLights{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
    source=sources
)

Default(library)

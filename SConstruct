import sys

# --- Default target ---
if not ARGUMENTS.get("target"):
    ARGUMENTS["target"] = "template_debug"

env = SConscript("godot-cpp/SConstruct")

# --- Set Flag for editor mode
if ARGUMENTS["target"] == "template_debug":
    env.Append(CPPDEFINES=['IN_EDITOR'])

# --- Compiler-specific flags ---
# Works with all compilers, MSVC included
if env['CC'].lower().endswith("cl") or env['CXX'].lower().endswith("cl"):
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
    "include/",
    "openRGB/",
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

# =========================================================================
# Cleaning Obj files
# =========================================================================
Alias('clean-editor', [])

if 'clean-editor' in COMMAND_LINE_TARGETS:
    import os
    print("scons: Purging local editor object files strictly from sources...")
    
    # Resolve .cpp sources into .obj nodes
    obj_ext = ".obj" if is_windows else ".o"

    real_source_paths = [str(f) for f in sources]

    # Get folders
    source_dirs = set()
    for src_file in sources:
        dir_name = os.path.dirname(str(src_file))
        if dir_name: # Keep only valid paths
            source_dirs.add(dir_name)
    
    # Remove only obj files matching "editor"
    for folder in source_dirs:
        if os.path.exists(folder):
            for file_name in os.listdir(folder):
                if "editor" in file_name and file_name.endswith(obj_ext):
                    full_path = os.path.join(folder, file_name)
                    if os.path.exists(full_path):
                        try:
                            os.remove(full_path)
                            print(f"Removed editor object: {full_path}")
                        except OSError as e:
                            print(f"Error deleting {full_path}: {e}")
                    
    # Exit so it doesn't trigger build sequence
    Exit(0)
#==============================================================================

# --- Documentation
if env["target"] in ["editor", "template_debug"]:
    try:
        doc_data = env.GodotCPPDocData("godot-cpp/gen/doc_data.gen.cpp", source=Glob("doc_classes/*.xml"))
        sources.append(doc_data)
    except AttributeError:
        print("Not including class reference as we're targeting a pre-4.3 baseline.")

# --- Build shared library ---
library = env.SharedLibrary(
    "demo/bin/GDGameLights{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
    source=sources
)

Default(library)

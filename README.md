# GDGamepadLights
#### OpenRGB GDExtension Integration for I/O Lighting Control

This project integrates **OpenRGB** into a Godot GDExtension to enable direct control of RGB lighting on supported gamepads and other I/O devices. It is built using the [OpenRGB C++ SDK](https://github.com/Youda008/OpenRGB-cppSDK) and adapted to work with a modified dependency stack based on **CppUtils** and **CppUtils-Network**.

Several patches where made to openRGB. Files [ProtocolCommon.hpp](openRGB/src/ProtocolCommon.hpp), [DeviceInfo.cpp](openRGB/src/DeviceInfo.cpp), [ProtocolMessages.cpp](openRGB/src/ProtocolMessages.cpp) and [NetAddress.cpp](openRGB/external/CppUtils-Network/NetAddress.cpp) where updated to use the new [BinaryStream.hpp](openRGB/external/CppUtils-Essential/BinaryStream.hpp) from the extarnal libraries required. Patches are identified with comments.

Got it — here is **GDGamepadLights rewritten in the exact same structure and style as your GDDraco example, with no deviations**:

---

## Index

- [Features](#features)
- [How to Use](#how-to-use)
  - [Prerequisites](#prerequisites)
  - [1. Download the Latest Release](#1-download-the-latest-release)
  - [2. Add it to your Project](#2-add-it-to-your-project)
  - [3. Done](#3-done)
- [Developer Build](#developer-build)
  - [Prerequisites](#prerequisites-1)
  - [1. Clone the Repository](#1-clone-the-repository)
  - [2. Building](#2-building)
  - [3. Testing](#3-testing)
- [License](#license)
- [Credits](#credits)

---

## Features

- Full integration with OpenRGB for I/O devices lighting control.
- Supports RGB gamepads and compatible hardware.
- Uses OpenRGB C++ SDK for communication with OpenRGB server.
- Updated to work with modern CppUtils networking and utility libraries.

---

## How to Use

### Prerequisites

- **[Godot 4.5](https://godotengine.org/)**  
  _⚠️ This extension was tested with **Godot 4.5**. It may work with other 4.x versions, but compatibility is not guaranteed._

### 1. Download the Latest Release
You can find the latest release for Windows and Linux in [Releases](https://github.com/itslebi/GDGamepadLights/releases).  
MacOS users must build the extension manually using the developer build instructions.

### 2. Add it to your project
Unzip the release archive and place the resulting folder into your main Godot project directory.

### 3. Download Open RGB 
> ⚠️Your Players must follow steps 3 and 4 in order for it to be active during gameplay!

Go to [OpenRGB official website](https://openrgb.org/) and download OpenRGB for your OS.

### 4. Start OpenRGB Server
> ⚠️Each time you want to use GDGameLights you need to start the OpenRGB server on the correct port!

Open OpenRGB's app, navigate to **SDK server** and start the server on the port that you are using on your project! Default port for the extension is **6742**.
<img width="524" height="371" alt="image" src="https://github.com/user-attachments/assets/1b9925e5-674a-461e-9f1d-d08a263f078e" />

### 5. Done
You can now control supported RGB gamepads through OpenRGB from your Godot project.

---

## Developer Build

### Prerequisites

- **[Godot 4.5](https://godotengine.org/)**  
  _⚠️ This extension was tested with **Godot 4.5**. It may work with other 4.x versions, but compatibility is not guaranteed._  
- C++ build environment (GCC / Clang / MSVC)
- [Python](https://www.python.org/)
- [SCons](https://scons.org/) (used for building)

_⚠️ [Godot CPP](https://github.com/godotengine/godot-cpp) is required and included in the project source._
_⚠️ OpenRGB SDK and CppUtils dependencies are included in modified form. Original sources used: [OpenRGB C++ SDK](https://github.com/Youda008/OpenRGB-cppSDK/tree/d2e40fa089d2e5160c1220b51bab62b35b61d4a5), [CppUtils-Essential](https://github.com/Youda008/CppUtils-Essential/commit/691850a7c5274870f839b69017896190f4287231), [CppUtils-Network](https://github.com/Youda008/CppUtils-Network/tree/12fa22ef505c76e36174f1854cfa495cb9770247)_

### 1. Clone the Repository

```bash
git clone https://github.com/itslebi/GDGamepadLights
cd GDGamepadLights
````

### 2. Building

Run the following command inside the main `GDGamepadLights` folder:

```bash
scons target=template_release
```

*⚠️ If OpenRGB SDK or dependencies fail to build, refer to their respective documentation.*

> ⚙️ This will build the GDExtension and output the compiled binary (`.dll` or `.so`) in the `bin/` directory.

Additionally, the following definition is required in `SConstruct`:
> The provided `SConstruct` already includes it.

```cpp
env.Append(CPPDEFINES=[('critical_error', 'CRITICAL_ERROR')])
```

This ensures compatibility with legacy `CppUtils-Essential` calls.

---

### 3. Testing

*⚠️ Verify that the compiled binary is correctly referenced in the `.gdextension` file.*

Use the included demo project:

* Open the project in Godot
* Ensure OpenRGB is running with SDK enabled
* Test device lighting control via supported gamepads

---

## License

**GDGamepadLights** is open source and licensed under the **MIT License**.
You are free to use, modify, and distribute this software as long as the original license and copyright notice are included.

Please note:

* [OpenRGB](https://openrgb.org/) is licensed under a GPL-compatible license
* [OpenRGB C++ SDK](https://github.com/Youda008/OpenRGB-cppSDK) is licensed under MIT
* [CppUtils-Essential](https://github.com/Youda008/CppUtils-Essential) and [CppUtils-Network](https://github.com/Youda008/CppUtils-Network) do not have a public license but are part of the official dependencies of OpenRGB C++ SDK

---

## Credits

**This extension was made by me alone but it would not be possible without the below open source projects!**

* [Godot Engine](https://godotengine.org/) — the engine it is built for
* [OpenRGB](https://openrgb.org/) — RGB lighting control system
* [OpenRGB C++ SDK](https://github.com/Youda008/OpenRGB-cppSDK) — SDK for device communication
* [CppUtils-Essential](https://github.com/Youda008/CppUtils-Essential) — utility framework
* [CppUtils-Network](https://github.com/Youda008/CppUtils-Network) — networking utilities

> © 2025 [@itslebi](https://github.com/itslebi) — Contributions welcome!

# GDGamepadLights

Several patches where made to openRGB. Files ProtocolCommon.hpp, DeviceInfo.cpp, ProtocolMessages.cpp and NetAddress.cpp where updated to use the new BinaryStream.hpp from the extarnal libraries required. Patches are identified with comments. The line ```env.Append(CPPDEFINES=[('critical_error', 'CRITICAL_ERROR')])``` was added to the SConstruct file so that the calls to critical_error can remain the same as in the old version of the CppUtils-Essential library.
Using https://github.com/Youda008/OpenRGB-cppSDK
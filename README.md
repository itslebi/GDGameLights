# GDGamepadLights

Several patches where made to openRGB. Files ProtocolCommon.hpp, DeviceInfo.cpp, ProtocolMessages.cpp and NetAddress.cpp where updated to use the new BinaryStream.hpp from the extarnal libraries required. Patches are identified with comments. The line ```env.Append(CPPDEFINES=[('critical_error', 'CRITICAL_ERROR')])``` was added to the SConstruct file so that the calls to critical_error can remain the same as in the old version of the CppUtils-Essential library.
Using https://github.com/Youda008/OpenRGB-cppSDK

https://github.com/Youda008/CppUtils-Essential/commit/691850a7c5274870f839b69017896190f4287231
https://github.com/Youda008/OpenRGB-cppSDK/tree/d2e40fa089d2e5160c1220b51bab62b35b61d4a5
https://github.com/Youda008/CppUtils-Network/tree/12fa22ef505c76e36174f1854cfa495cb9770247
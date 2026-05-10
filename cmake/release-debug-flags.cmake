# O3 is default setting for CMake Release builds, override it by O2
if(${CMAKE_CXX_FLAGS_RELEASE} MATCHES "-O3")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
endif()
# work around android toolchain setting -Oz for only this case and -O2 for everything else
if(ANDROID_ABI MATCHES "^armeabi" AND ANDROID_ARM_MODE STREQUAL thumb)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
endif()

# enable asserts in RelWithDebInfo builds
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -UNDEBUG")

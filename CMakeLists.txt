cmake_minimum_required(VERSION 3.10)

project(stalks_evdev)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-Wall -Ofast")

include_directories(
    src
    ../scs_sdk_1_14/include
    /usr/include/libevdev-1.0
)

add_library(stalks_evdev SHARED
    src/main.cpp
    src/input.cpp
    src/globals.cpp
    src/globals.h
    src/telemetry.cpp
)

add_executable(test
    test/test.cpp
)

target_link_libraries(stalks_evdev
    evdev
)


target_link_libraries(test
    stalks_evdev
    evdev
)

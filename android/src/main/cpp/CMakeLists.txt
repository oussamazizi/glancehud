# Builds the Android shared library `libglancehud.so`:
#   pure core (../../../../core)  +  JNI bridge  +  Android /proc providers.
cmake_minimum_required(VERSION 3.22)
project(glancehud_android LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Reuse the exact same core sources the desktop/CI build uses.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../../core
                 ${CMAKE_BINARY_DIR}/glancehud_core)

add_library(glancehud SHARED
    glance_jni.cpp
    providers/CpuProvider.cpp
    providers/RamProvider.cpp
    providers/NetProvider.cpp
)

target_include_directories(glancehud PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

find_library(log-lib log)
target_link_libraries(glancehud PRIVATE glancehud::core ${log-lib})

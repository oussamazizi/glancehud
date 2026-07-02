# GlanceHUD core: pure C++20 engine, ZERO platform dependencies.
cmake_minimum_required(VERSION 3.22)

add_library(glancehud_core STATIC
    src/Version.cpp
    src/SnapshotStore.cpp
    src/FpsCalculator.cpp
    src/MetricsEngine.cpp
    src/PerfOverlay.cpp
    src/Api.cpp
)
add_library(glancehud::core ALIAS glancehud_core)

target_include_directories(glancehud_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(glancehud_core PUBLIC cxx_std_20)

find_package(Threads REQUIRED)
target_link_libraries(glancehud_core PUBLIC Threads::Threads)

if(MSVC)
    target_compile_options(glancehud_core PRIVATE /W4)
    if(GLANCEHUD_WARNINGS_AS_ERRORS)
        target_compile_options(glancehud_core PRIVATE /WX)
    endif()
else()
    target_compile_options(glancehud_core PRIVATE -Wall -Wextra -Wpedantic)
    if(GLANCEHUD_WARNINGS_AS_ERRORS)
        target_compile_options(glancehud_core PRIVATE -Werror)
    endif()
endif()

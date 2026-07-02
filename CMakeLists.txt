cmake_minimum_required(VERSION 3.22)

project(GlanceHUD
    VERSION 0.1.0
    DESCRIPTION "Lightweight cross-platform native performance overlay"
    HOMEPAGE_URL "https://github.com/oussamazizi/glancehud"
    LANGUAGES CXX)

# ---- Global standards ---------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---- Options ------------------------------------------------------------------
option(GLANCEHUD_BUILD_TESTS "Build the unit tests" ON)
option(GLANCEHUD_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

# ---- Modules ------------------------------------------------------------------
add_subdirectory(core)

if(GLANCEHUD_BUILD_TESTS)
    enable_testing()
    add_subdirectory(core/tests)
endif()

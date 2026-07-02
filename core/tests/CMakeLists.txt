# Unit tests for glancehud_core using GoogleTest (fetched at configure time).
include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2
)
# Keep gtest linkage consistent with the project on all platforms.
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(glancehud_core_tests
    smoke_test.cpp
    engine_test.cpp
)

# Tests exercise internal engine classes, so they need the private src headers.
target_include_directories(glancehud_core_tests
    PRIVATE ${CMAKE_SOURCE_DIR}/core/src
)

target_link_libraries(glancehud_core_tests
    PRIVATE
        glancehud::core
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(glancehud_core_tests)

cmake_minimum_required(VERSION 3.18)
project(DiaryApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(SOURCES
    src/entry.cpp
    src/account.cpp
    src/storage.cpp
    src/search.cpp
    src/ui.cpp
    src/app.cpp
    src/main.cpp
)

# Include directories
include_directories(include)

# Build executable
add_executable(diaryapp ${SOURCES})

# On macOS, link against system for SHA-256 (CommonCrypto)
if(APPLE)
    target_link_libraries(diaryapp PRIVATE)
endif()

# Enable testing
enable_testing()

# Fetch Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Test sources
set(TEST_SOURCES
    tests/tests.cpp
    src/entry.cpp
    src/account.cpp
    src/storage.cpp
    src/search.cpp
)

add_executable(run_tests ${TEST_SOURCES})
target_include_directories(run_tests PRIVATE include)
target_link_libraries(run_tests gtest_main gmock_main)

include(GoogleTest)
gtest_discover_tests(run_tests)

cmake_minimum_required(VERSION 3.10)
project(dissonance_core)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

option(BUILD_NODE_ADDON "Build the Node.js addon with CMake (node_api.h required)" OFF)
option(BUILD_CLI "Build the CLI executable (requires WavGUI)" OFF)

if (BUILD_NODE_ADDON)
    # Include node-addon-api headers
    include_directories(${CMAKE_SOURCE_DIR}/node_modules/node-addon-api)

    # If you want CMake to find node_api.h, point NODE_INCLUDE_DIR to your Node headers, e.g.
    #   cmake -S . -B cmake-build -DBUILD_NODE_ADDON=ON -DNODE_INCLUDE_DIR="C:/Users/<you>/AppData/Local/node-gyp/Cache/<ver>/include/node"
    if (DEFINED NODE_INCLUDE_DIR)
        include_directories(${NODE_INCLUDE_DIR})
    endif()

    add_library(dissonance_core SHARED
            src/addon.cpp
            src/AddonHelpers.cpp
            src/GainProcessor.cpp
            src/WavUtils.cpp
            src/WavProcessor.cpp
            src/WindowFunctions.cpp
            src/FFTProcessor.cpp
    )

    # Build actual .node file for Node.js
    set_target_properties(dissonance_core PROPERTIES
            PREFIX ""
            SUFFIX ".node"
    )
endif()

# Standalone CLI for inspecting WAV files (optional)
if (BUILD_CLI)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

    set(DISSONANCE_SOURCES
            src/main.cpp
            src/WavGUI.cpp
            src/GainProcessor.cpp
            src/WavUtils.cpp
            src/WavProcessor.cpp
            src/WindowFunctions.cpp
            src/FFTProcessor.cpp
    )

    add_executable(dissonance.pre-alpha ${DISSONANCE_SOURCES})
endif()

# Optional GoogleTest support when tests are present
enable_testing()
file(GLOB TEST_SOURCES tests/*.cpp)
find_package(GTest QUIET)
if (GTest_FOUND AND TEST_SOURCES)
        add_executable(runTests ${TEST_SOURCES} src/WavUtils.cpp src/WavProcessor.cpp src/GainProcessor.cpp src/WindowFunctions.cpp src/FFTProcessor.cpp)
    set_target_properties(runTests PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    target_link_libraries(runTests PRIVATE GTest::GTest GTest::Main)
    add_test(NAME runTests COMMAND runTests)
endif()

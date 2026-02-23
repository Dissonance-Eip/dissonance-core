cmake_minimum_required(VERSION 3.10)
project(dissonance-core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_NODE_ADDON "Build the Node.js addon with CMake (node_api.h required)" OFF)
option(BUILD_CLI "Build the CLI executable (requires WavGUI)" ON)

set(DISSONANCE_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)

set(DISSONANCE_AUDIO_SOURCES
        src/audio/GainProcessor.cpp
        src/utils/WavUtils.cpp
        src/audio/WavProcessor.cpp
        src/audio/WindowFunctions.cpp
        src/audio/FFTProcessor.cpp
)

# -------------------------
# Node.js addon (.node)
# -------------------------
if(BUILD_NODE_ADDON)
        add_library(dissonance_core SHARED
                src/addon/addon.cpp
                src/addon/AddonHelpers.cpp
                ${DISSONANCE_AUDIO_SOURCES}
        )

        target_include_directories(dissonance_core PRIVATE
                ${DISSONANCE_INCLUDE_DIR}
                ${CMAKE_SOURCE_DIR}/node_modules/node-addon-api
        )

        # If you want CMake to find node_api.h, point NODE_INCLUDE_DIR to your Node headers, e.g.
        # cmake -S . -B cmake-build -DBUILD_NODE_ADDON=ON -DNODE_INCLUDE_DIR="C:/Users/<you>/AppData/Local/node-gyp/Cache/<ver>/include/node"
        if(DEFINED NODE_INCLUDE_DIR)
                target_include_directories(dissonance_core PRIVATE ${NODE_INCLUDE_DIR})
        endif()

        set_target_properties(dissonance_core PROPERTIES
                PREFIX ""
                SUFFIX ".node"
        )
endif()

# -------------------------
# Standalone CLI executable
# -------------------------
if(BUILD_CLI)
        add_executable(dissonance.core
                src/cli/main.cpp
                src/cli/Commands.cpp
                src/cli/WavGUI.cpp
                ${DISSONANCE_AUDIO_SOURCES}
        )

        target_include_directories(dissonance.core PRIVATE ${DISSONANCE_INCLUDE_DIR})

        set_target_properties(dissonance.core PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
endif()

# -------------------------
# Unit tests (GoogleTest)
# -------------------------
enable_testing()

file(GLOB TEST_SOURCES tests/*.cpp)
find_package(GTest QUIET)

if(GTest_FOUND AND TEST_SOURCES)
        add_executable(runTests
                ${TEST_SOURCES}
                ${DISSONANCE_AUDIO_SOURCES}
        )

        target_include_directories(runTests PRIVATE ${DISSONANCE_INCLUDE_DIR})

        set_target_properties(runTests PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )

        target_link_libraries(runTests PRIVATE GTest::GTest GTest::Main)
        add_test(NAME runTests COMMAND runTests)
endif()

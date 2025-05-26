cmake_minimum_required(VERSION 3.10)
project(dissonance.pre-alpha)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

set(SOURCES
    src/main.cpp
    src/WavGui.cpp
    include/WavParser.hpp
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_executable(dissonance.pre-alpha ${SOURCES})

# Enable testing and add Google Test
enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Add test executable
file(GLOB TEST_SOURCES tests/*.cpp)
if(TEST_SOURCES)
    add_executable(runTests ${TEST_SOURCES} src/WavGui.cpp)
    set_target_properties(runTests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    target_link_libraries(runTests PRIVATE GTest::GTest GTest::Main pthread)
    add_test(NAME runTests COMMAND runTests)
endif()

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

# Enable testing and fetch Google Test only if building tests
include(CTest)
if(BUILD_TESTING)
    include(FetchContent)
    FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/refs/heads/main.zip
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()
    include_directories(${gtest_SOURCE_DIR}/include)

    # Add test executable
    file(GLOB TEST_SOURCES tests/*.cpp)
    if(TEST_SOURCES)
        add_executable(runTests ${TEST_SOURCES} src/WavGui.cpp)
        set_target_properties(runTests PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
        target_link_libraries(runTests PRIVATE gtest gtest_main pthread)
        add_test(NAME runTests COMMAND runTests)
    endif()
endif()

cmake_minimum_required(VERSION 3.11)
project(dissonance-core LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_NODE_ADDON "Build the Node.js addon (requires node_api.h)" OFF)
option(BUILD_CLI "Build the CLI executable" ON)

set(DISSONANCE_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)

include(FetchContent)
FetchContent_Declare(
        kissfft_src
        GIT_REPOSITORY https://github.com/mborgerding/kissfft.git
        GIT_TAG        master
        SOURCE_SUBDIR  "_none_"
)
FetchContent_MakeAvailable(kissfft_src)

add_library(kissfft STATIC ${kissfft_src_SOURCE_DIR}/kiss_fft.c)
target_include_directories(kissfft PUBLIC ${kissfft_src_SOURCE_DIR})
set_target_properties(kissfft PROPERTIES LINKER_LANGUAGE C)
target_compile_options(kissfft PRIVATE
        $<$<C_COMPILER_ID:MSVC>:/wd4267>
        $<$<C_COMPILER_ID:GNU,Clang>:-w>
)

set(DISSONANCE_SOURCES
        src/utils/WavParser.cpp
        src/utils/WavUtils.cpp
        src/audio/GainStage.cpp
        src/audio/Pipeline.cpp
        src/audio/WavProcessor.cpp
        src/audio/WindowedFFTStage.cpp
        src/audio/WindowFunctions.cpp
        src/audio/FFTProcessor.cpp
)

if(BUILD_NODE_ADDON)
        add_library(dissonance_core SHARED
                src/addon/addon.cpp
                src/addon/AddonHelpers.cpp
                ${DISSONANCE_SOURCES}
        )
        target_include_directories(dissonance_core PRIVATE
                ${DISSONANCE_INCLUDE_DIR}
                ${CMAKE_SOURCE_DIR}/node_modules/node-addon-api
        )
        if(DEFINED NODE_INCLUDE_DIR)
                target_include_directories(dissonance_core PRIVATE ${NODE_INCLUDE_DIR})
        endif()
        target_link_libraries(dissonance_core PRIVATE kissfft)
        set_target_properties(dissonance_core PROPERTIES PREFIX "" SUFFIX ".node")
endif()

if(BUILD_CLI)
        add_executable(dissonance.core
                src/cli/main.cpp
                src/cli/Commands.cpp
                src/cli/ConsolePrinter.cpp
                ${DISSONANCE_SOURCES}
        )
        target_include_directories(dissonance.core PRIVATE ${DISSONANCE_INCLUDE_DIR})
        target_link_libraries(dissonance.core PRIVATE kissfft)
        set_target_properties(dissonance.core PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
endif()

enable_testing()
file(GLOB TEST_SOURCES tests/*.cpp)
list(FILTER TEST_SOURCES EXCLUDE REGEX ".*/bench_main\\.cpp$")
find_package(GTest QUIET)

if(GTest_FOUND AND TEST_SOURCES)
        add_executable(runTests ${TEST_SOURCES} ${DISSONANCE_SOURCES})
        target_include_directories(runTests PRIVATE ${DISSONANCE_INCLUDE_DIR})
        target_link_libraries(runTests PRIVATE kissfft GTest::GTest GTest::Main)
        set_target_properties(runTests PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
        add_test(NAME runTests COMMAND runTests)
endif()

option(BUILD_BENCH "Build benchmark executable" OFF)
if(BUILD_BENCH)
        add_executable(dissonance.bench tests/bench_main.cpp ${DISSONANCE_SOURCES})
        target_include_directories(dissonance.bench PRIVATE ${DISSONANCE_INCLUDE_DIR})
        target_link_libraries(dissonance.bench PRIVATE kissfft)
        set_target_properties(dissonance.bench PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
endif()

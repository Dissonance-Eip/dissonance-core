cmake_minimum_required(VERSION 3.10)
project(dissonance.pre-alpha)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

set(SOURCES
    src/main.cpp
    src/WavGui.cpp
    include/WavParser.hpp
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(dissonance.pre-alpha ${SOURCES})
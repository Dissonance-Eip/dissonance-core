# Dissonance Pre-Alpha

## Overview

This project is designed to open a `.wav` file, extract all metadata and audio data from it, and display the information in the terminal.

## Build Instructions

1. Ensure you have CMake installed (version 3.10 or higher).
2. Navigate to the project directory.
3. Create a build directory: `mkdir build && cd build`
4. Run CMake to configure the project: `cmake ..`
5. Build the project: `make`

## Usage

After building the project, you can run the executable with a `.wav` file as input:

``` bash
./dissonance.pre-alpha <path_to_wav_file>
```

## Files

- `CMakeLists.txt`: Configuration file for CMake to build the project.
- `README.md`: Project documentation.
- `src/main.cpp`: The main entry point of the application.
- `src/WavParser.cpp`: Implementation of the `Parser` class.
- `include/WavParser.hpp`: Header file for the `Parser` class.
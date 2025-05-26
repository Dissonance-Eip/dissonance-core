# Technology Environment

This document outlines the technological foundation of the `dissonance-core` project.

---

## Core Technologies

| Component        | Technology |
|------------------|-----------|
| Audio Processing | C++ |
| Build System     | CMake |
| CLI Interface    | C++ |
| Unit Testing     | Google Test |
| CI/CD            | GitHub Actions |
| Design Prototyping | Figma (for future frontend) |

---

## Libraries

- **JUCE** (planned): For future real-time integration
- **RtAudio** (planned): For streaming-based audio testing
- **GTest**: For unit and integration testing

All libraries are actively maintained and open-source.

---

## Infrastructure

- **Source Control**: GitHub
- **CI/CD**: GitHub Actions
- **Testing Framework**: Google Test + `ctest`
- **Package Management**: System package manager (APT) or CMake’s `find_package()`

---

## Hardware Requirements

For development:
- Linux/macOS/Windows machine
- C++17-compatible compiler (g++, clang++)
- Minimum 8GB RAM

For users:
- Standard desktop machine
- No GPU required for CLI version

---

## Future Plans

- Add precompiled binary releases to GitHub Releases
- Real-time preview pipeline with JUCE (desktop app repo)
- Plugin build system (VST/AU) for integration with DAWs

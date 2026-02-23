# Dissonance Core

Native C++ audio core that provides:
- a Node addon (`.node`) for app integration
- a CLI executable (`dissonance.core.exe`) for local WAV testing

## Local builds (what to run)

From the `core` folder:

- Build addon only (for Node usage):
  - `npm run build:addon`
  - Output: `build/Release/dissonance_core.node`

- Build addon + platform artifact name (for release packaging):
  - `npm run build:addon:dist`
  - Output: `dist/dissonance_core-<platform>-<arch>.node`

- Build CLI executable (Debug):
  - `npm run build:cli:debug`
  - Output: `cmake-build/Debug/dissonance.core.exe`

- Build CLI executable (Release):
  - `npm run build:cli:release`
  - Output: `cmake-build/Release/dissonance.core.exe`

- Build both for local testing (addon artifact + Debug CLI):
  - `npm run build:local`

## Run CLI locally

- `./cmake-build/Debug/dissonance.core.exe info ./test_files/sound.wav`
- `./cmake-build/Release/dissonance.core.exe info ./test_files/sound.wav`

## C++ formatting

- Format: `npm run format:cpp`
- Check only: `npm run format:cpp:check`

## Workflows

### `ci.yml` (C++ quality/build)
- `cppcheck`: static analysis on `src`
- `clang-format-check`: enforces `.clang-format` via `npm run format:cpp:check`
- `cmake-tests`: configures and builds with CMake, then runs CTest

### `release-addon.yml` (addon release)
- `addon-checks`: installs deps, builds addon, smoke-loads `.node`, runs JS addon tests
- `build-addon`: matrix build for Win/Linux/macOS and prepares `dist/*.node`
- `release`: publishes `.node` artifacts and dispatches UI sync

## Notes

- Local addon build uses `node-gyp` (`binding.gyp`).
- Local CLI build uses CMake (`CMakeLists.txt`).
- If `cmake` is not on PATH, set `CMAKE_EXE` to the full executable path.
- If `clang-format` is not on PATH, set `CLANG_FORMAT` to the full executable path.

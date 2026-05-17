# Dissonance Core

Native C++ audio core that provides:
- A Node.js addon (`.node`) consumed by the Electron UI
- A CLI executable (`dissonance.core`) for local testing and F3 evidence

## Addon contract

```js
process(inputPath: string): Promise<{ ok: boolean, processedPath: string }>
```

The addon applies the full protection pipeline (spectral low-pass filter → gain)
and writes the result next to the input file as `<stem>-processed.wav`.

## Local builds

From the `core` folder:

### Addon

```bash
npm run build               # build the .node addon (alias: build:addon)
npm run build:addon:dist    # build + rename to dist/dissonance_core-<platform>-<arch>.node
```

Output: `build/Release/dissonance_core.node`

### CLI

```bash
npm run build:cli:debug     # Debug build   → cmake-build/Debug/dissonance.core
npm run build:cli:release   # Release build → cmake-build/Release/dissonance.core
npm run build:local         # addon:dist + cli:debug in one step
```

## Run the CLI

```bash
cmake-build/Debug/dissonance.core info <file.wav>
cmake-build/Debug/dissonance.core process <file.wav> --gain 0.5 --output out.wav
cmake-build/Debug/dissonance.core fft <file.wav> --full --sort
```

## Testing

C++ unit tests are run via CTest (part of the `cmake-tests` CI job):

```bash
cmake -S . -B cmake-build -DBUILD_CLI=ON -DBUILD_NODE_ADDON=OFF
cmake --build cmake-build
ctest --test-dir cmake-build --output-on-failure
```

A WAV fixture for local addon smoke-testing lives at `test_files/sound.wav`.

## Code quality

```bash
npm run format:cpp          # auto-format all C++ files with clang-format
npm run format:cpp:check    # check only (run by CI)
npm run cppcheck            # run cppcheck static analysis locally
```

**Always run `npm run format:cpp` before pushing** — the CI will fail otherwise.

## Workflows

### `ci.yml` — Core Quality Checks
Runs on every push and PR:
- `cppcheck` — static analysis on `src/`
- `clang-format-check` — enforces `.clang-format` style via `npm run format:cpp:check`
- `cmake-tests` — CMake configure, build, and CTest

### `release-addon.yml` — Addon Release
Triggered on push to `dev` or a version tag:
- `addon-checks` — installs deps, builds addon, smoke-loads `.node`
- `build-addon` — matrix build for Windows / Linux / macOS arm64, produces `dist/*.node`
- `release` — publishes `.node` artifacts to GitHub Releases and dispatches UI sync

## Notes

- Addon build uses `node-gyp` (`binding.gyp`).
- CLI build uses CMake (`CMakeLists.txt`). Set `CMAKE_EXE` if `cmake` is not on PATH.
- Set `CLANG_FORMAT` if `clang-format` is not on PATH.
- Set `CMAKE_BUILD_DIR` to override the default `cmake-build/` output directory.

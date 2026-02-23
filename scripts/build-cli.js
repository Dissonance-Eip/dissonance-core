const { spawnSync } = require('child_process');
const fs = require('fs');

const configArg = process.argv.find((arg) => arg.startsWith('--config='));
const config = configArg ? configArg.split('=')[1] : 'Debug';
const buildDir = process.env.CMAKE_BUILD_DIR || 'cmake-build';

function findCmake() {
  if (process.env.CMAKE_EXE && fs.existsSync(process.env.CMAKE_EXE)) {
    return process.env.CMAKE_EXE;
  }

  const candidates = [
    'cmake',
    'cmake.exe',
    'C:/Program Files/CMake/bin/cmake.exe',
    'C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe',
  ];

  for (const candidate of candidates) {
    if (candidate.includes('/') || candidate.includes('\\')) {
      if (fs.existsSync(candidate)) {
        return candidate;
      }
      continue;
    }

    const probe = spawnSync(candidate, ['--version'], { stdio: 'ignore', shell: false });
    if (probe.status === 0) {
      return candidate;
    }
  }

  return null;
}

function run(cmd, args) {
  const result = spawnSync(cmd, args, { stdio: 'inherit', shell: false });
  if (result.status !== 0) {
    process.exit(result.status || 1);
  }
}

const cmake = findCmake();
if (!cmake) {
  console.error('cmake not found. Install CMake or set CMAKE_EXE to the executable path.');
  process.exit(1);
}

run(cmake, ['-S', '.', '-B', buildDir, '-DBUILD_NODE_ADDON=OFF', '-DBUILD_CLI=ON']);
run(cmake, ['--build', buildDir, '--config', config]);

console.log(`Built CLI with ${cmake} (${config}) in ${buildDir}.`);

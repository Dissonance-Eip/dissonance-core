const { spawnSync } = require('child_process');
const fs = require('fs');
const path = require('path');

function findCppcheck() {
  if (process.env.CPPCHECK && fs.existsSync(process.env.CPPCHECK)) {
    return process.env.CPPCHECK;
  }

  const candidates = [
    'cppcheck',
    'cppcheck.exe',
    'C:/Program Files/Cppcheck/cppcheck.exe',
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

const cppcheck = findCppcheck();
if (!cppcheck) {
  console.error('cppcheck not found. Install it with: winget install --id Cppcheck.Cppcheck -e');
  process.exit(1);
}

const srcDir = path.resolve(process.cwd(), 'src');
if (!fs.existsSync(srcDir)) {
  console.log('No src directory found. Skipping cppcheck.');
  process.exit(0);
}

const args = [
  '--enable=warning,style,performance,portability',
  '--std=c++17',
  '--language=c++',
  '--error-exitcode=1',
  '--quiet',
  '-I',
  'include',
  'src',
];

const result = spawnSync(cppcheck, args, { stdio: 'inherit', shell: false });
if (result.status !== 0) {
  process.exit(result.status || 1);
}

console.log(`cppcheck passed with ${cppcheck}`);

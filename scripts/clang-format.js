const { spawnSync } = require('child_process');
const fs = require('fs');
const path = require('path');

const mode = process.argv.includes('--check') ? 'check' : 'write';

function findClangFormat() {
  if (process.env.CLANG_FORMAT && fs.existsSync(process.env.CLANG_FORMAT)) {
    return process.env.CLANG_FORMAT;
  }

  const candidates = [
    'clang-format',
    'clang-format.exe',
    'C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/VC/Tools/Llvm/x64/bin/clang-format.exe',
    'C:/Program Files/Microsoft Visual Studio/2022/BuildTools/VC/Tools/Llvm/x64/bin/clang-format.exe',
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

function collectFiles(dir, out) {
  const entries = fs.readdirSync(dir, { withFileTypes: true });
  for (const entry of entries) {
    const fullPath = path.join(dir, entry.name);
    if (entry.isDirectory()) {
      collectFiles(fullPath, out);
      continue;
    }

    if (/\.(c|cc|cpp|h|hpp)$/i.test(entry.name)) {
      out.push(fullPath);
    }
  }
}

const clangFormat = findClangFormat();
if (!clangFormat) {
  console.error('clang-format not found. Install LLVM clang-format or set CLANG_FORMAT to the executable path.');
  process.exit(1);
}

const files = [];
for (const root of ['src', 'include', 'tests']) {
  const rootPath = path.resolve(process.cwd(), root);
  if (fs.existsSync(rootPath)) {
    collectFiles(rootPath, files);
  }
}

if (files.length === 0) {
  console.log('No C/C++ files found.');
  process.exit(0);
}

const args = mode === 'check' ? ['-n', '--Werror', ...files] : ['-i', ...files];
const result = spawnSync(clangFormat, args, { stdio: 'inherit', shell: false });

if (result.status !== 0) {
  process.exit(result.status || 1);
}

console.log(`${mode === 'check' ? 'Checked' : 'Formatted'} ${files.length} files with ${clangFormat}`);

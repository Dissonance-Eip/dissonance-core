const fs = require('fs');
const path = require('path');

const src = path.join('build', 'Release', 'dissonance_core.node');

if (!fs.existsSync(src)) {
  console.error('Missing', src);
  process.exit(1);
}

const platform = `${process.platform}-${process.arch}`;
const destDir = 'dist';
const dest = path.join(destDir, `dissonance_core-${platform}.node`);

fs.mkdirSync(destDir, { recursive: true });
fs.copyFileSync(src, dest);

// On macOS, apply an ad-hoc code signature so Electron (hardened runtime)
// can load the binary without a CODESIGNING kill.
if (process.platform === 'darwin') {
  const { spawnSync } = require('child_process');
  const result = spawnSync('codesign', ['--sign', '-', '--force', dest]);
  if (result.status !== 0) {
    console.warn('codesign warning:', result.stderr && result.stderr.toString());
  }
}

console.log('Prepared', dest);

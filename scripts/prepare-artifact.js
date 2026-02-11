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

console.log('Prepared', dest);

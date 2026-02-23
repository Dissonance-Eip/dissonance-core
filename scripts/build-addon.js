const { spawnSync } = require('child_process');
const fs = require('fs');
const nodeGypCli = require.resolve('node-gyp/bin/node-gyp.js');

function run(args) {
  const result = spawnSync(process.execPath, [nodeGypCli, ...args], {
    stdio: 'inherit',
    shell: false,
  });
  if (result.status !== 0) {
    process.exit(result.status || 1);
  }
}

run(['configure']);

const bindingSolution = 'build/binding.sln';
if (process.platform === 'win32' && fs.existsSync(bindingSolution)) {
  run(['build', `--solution=${bindingSolution}`]);
} else {
  run(['build']);
}

console.log('Addon build completed.');
